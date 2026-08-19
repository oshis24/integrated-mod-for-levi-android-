#include "levi/modules/ItemPhysics.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"
#include "levi/core/State.hpp"
#include "levi/minecraft/ItemRenderer.hpp"

#include <cmath>

namespace levi::modules {

namespace {

float length3(
    float x,
    float y,
    float z
) noexcept {
    return std::sqrt(
        x * x +
        y * y +
        z * z
    );
}

bool normalize3(
    float x,
    float y,
    float z,
    float& outX,
    float& outY,
    float& outZ
) noexcept {
    const float length =
        length3(
            x,
            y,
            z
        );

    if (length <= 0.000001f) {
        return false;
    }

    const float inverse =
        1.0f / length;

    outX = x * inverse;
    outY = y * inverse;
    outZ = z * inverse;

    return true;
}

} // namespace

bool ItemPhysics::initialize() noexcept {
    if (
        status_ != ModuleStatus::Disabled &&
        status_ != ModuleStatus::Failed
    ) {
        return true;
    }

    auto& runtime =
        levi::core::Runtime::instance();

    if (!runtime.isInitialized()) {
        status_ =
            ModuleStatus::WaitingForRuntime;

        return false;
    }

    if (
        !runtime.isSupportedMinecraftVersion()
    ) {
        status_ =
            ModuleStatus::Failed;

        return false;
    }

    active_ = this;

    levi::minecraft::ItemRenderer::
        setWorldTransformCallback(
            &ItemPhysics::
                worldTransformCallback
        );

    status_ =
        ModuleStatus::WaitingForTarget;

    return true;
}

void ItemPhysics::bindNativeTargets(
    std::uintptr_t setupAndRender,
    std::uintptr_t renderItemGroup
) noexcept {
    if (
        setupAndRender != 0 &&
        !setupHook_.installed()
    ) {
        setupHook_ =
            levi::memory::Hook(
                setupAndRender,
                reinterpret_cast<void*>(
                    &ItemPhysics::
                        setupAndRenderDetour
                )
            );

        if (!setupHook_.install()) {
            levi::core::Logger::warning(
                "ItemPhysics: setupAndRender hook failed"
            );
        }
    }

    if (
        renderItemGroup != 0 &&
        !groupHook_.installed()
    ) {
        groupHook_ =
            levi::memory::Hook(
                renderItemGroup,
                reinterpret_cast<void*>(
                    &ItemPhysics::
                        renderItemGroupDetour
                )
            );

        if (!groupHook_.install()) {
            levi::core::Logger::error(
                "ItemPhysics: renderItemGroup hook failed"
            );
        }
    }

    if (groupHook_.installed()) {
        status_ =
            ModuleStatus::Ready;
    }
}

void ItemPhysics::shutdown() noexcept {
    disable();

    groupHook_.remove();
    setupHook_.remove();

    levi::minecraft::ItemRenderer::
        setWorldTransformCallback(
            nullptr
        );

    orientationCache_ = {};
    frameCounter_ = 0;

    if (active_ == this) {
        active_ = nullptr;
    }

    status_ =
        ModuleStatus::Disabled;
}

void ItemPhysics::tick(
    float deltaTime
) noexcept {
    (void)deltaTime;
}

bool ItemPhysics::enable() noexcept {
    if (!groupHook_.installed()) {
        status_ =
            ModuleStatus::WaitingForTarget;

        return false;
    }

    enabled_ = true;

    levi::core::State::instance()
        .setItemPhysicsEnabled(true);

    status_ =
        ModuleStatus::Active;

    return true;
}

void ItemPhysics::disable() noexcept {
    enabled_ = false;

    levi::core::State::instance()
        .setItemPhysicsEnabled(false);

    if (
        status_ == ModuleStatus::Active
    ) {
        status_ =
            groupHook_.installed()
                ? ModuleStatus::Ready
                : ModuleStatus::
                    WaitingForTarget;
    }
}

bool ItemPhysics::enabled() const noexcept {
    return enabled_;
}

ModuleStatus
ItemPhysics::status() const noexcept {
    return status_;
}

bool ItemPhysics::hooksInstalled()
    const noexcept {
    return groupHook_.installed();
}

void ItemPhysics::setupAndRenderDetour(
    void* self,
    void* context
) noexcept {
    auto* module = active_;

    const auto original =
        module != nullptr
            ? reinterpret_cast<
                SetupAndRenderFn
            >(
                module->
                    setupHook_.original()
            )
            : nullptr;

    if (module != nullptr) {
        ++module->frameCounter_;

        if (
            (module->frameCounter_ &
             0x7F) == 0
        ) {
            module->expireOldEntries();
        }
    }

    if (original != nullptr) {
        original(
            self,
            context
        );
    }
}

void ItemPhysics::renderItemGroupDetour(
    void* self,
    void* context,
    void* worldItem,
    int state,
    bool flag,
    float value0,
    float value1
) noexcept {
    auto* module = active_;

    const auto original =
        module != nullptr
            ? reinterpret_cast<
                RenderItemGroupFn
            >(
                module->
                    groupHook_.original()
            )
            : nullptr;

    if (original == nullptr) {
        return;
    }

    if (
        module != nullptr &&
        !module->setupHook_.installed()
    ) {
        ++module->frameCounter_;
    }

    levi::minecraft::ItemRenderer::
        beginWorldItemRender(
            worldItem
        );

    original(
        self,
        context,
        worldItem,
        state,
        flag,
        value0,
        value1
    );

    levi::minecraft::ItemRenderer::
        endWorldItemRender();
}

void ItemPhysics::worldTransformCallback(
    void* worldItem,
    levi::minecraft::MatrixStack&
        matrixStack
) noexcept {
    auto* module = active_;

    if (
        module == nullptr ||
        !module->enabled_
    ) {
        return;
    }

    module->applyStableOrientation(
        worldItem,
        matrixStack
    );
}

void ItemPhysics::applyStableOrientation(
    void* worldItem,
    levi::minecraft::MatrixStack&
        matrixStack
) noexcept {
    if (worldItem == nullptr) {
        return;
    }

    auto* matrix =
        matrixStack.current();

    if (matrix == nullptr) {
        return;
    }

    OrientationEntry* entry =
        findOrCreateEntry(
            worldItem
        );

    if (entry == nullptr) {
        return;
    }

    const float scaleX =
        length3(
            matrix->m[0],
            matrix->m[1],
            matrix->m[2]
        );

    const float scaleY =
        length3(
            matrix->m[4],
            matrix->m[5],
            matrix->m[6]
        );

    const float scaleZ =
        length3(
            matrix->m[8],
            matrix->m[9],
            matrix->m[10]
        );

    if (
        scaleX <= 0.000001f ||
        scaleY <= 0.000001f ||
        scaleZ <= 0.000001f
    ) {
        return;
    }

    if (!entry->valid) {
        bool ok = true;

        ok &= normalize3(
            matrix->m[0],
            matrix->m[1],
            matrix->m[2],
            entry->basis[0],
            entry->basis[1],
            entry->basis[2]
        );

        ok &= normalize3(
            matrix->m[4],
            matrix->m[5],
            matrix->m[6],
            entry->basis[3],
            entry->basis[4],
            entry->basis[5]
        );

        ok &= normalize3(
            matrix->m[8],
            matrix->m[9],
            matrix->m[10],
            entry->basis[6],
            entry->basis[7],
            entry->basis[8]
        );

        if (!ok) {
            return;
        }

        entry->valid = true;
    }

    /*
     * Freeze orientation, but preserve the model's own
     * initial basis and current scale.
     *
     * No universal 90-degree correction.
     */
    matrix->m[0] =
        entry->basis[0] * scaleX;
    matrix->m[1] =
        entry->basis[1] * scaleX;
    matrix->m[2] =
        entry->basis[2] * scaleX;

    matrix->m[4] =
        entry->basis[3] * scaleY;
    matrix->m[5] =
        entry->basis[4] * scaleY;
    matrix->m[6] =
        entry->basis[5] * scaleY;

    matrix->m[8] =
        entry->basis[6] * scaleZ;
    matrix->m[9] =
        entry->basis[7] * scaleZ;
    matrix->m[10] =
        entry->basis[8] * scaleZ;

    entry->age =
        frameCounter_;
}

ItemPhysics::OrientationEntry*
ItemPhysics::findOrCreateEntry(
    void* key
) noexcept {
    OrientationEntry* freeEntry =
        nullptr;

    OrientationEntry* oldest =
        nullptr;

    for (auto& entry : orientationCache_) {
        if (
            entry.valid &&
            entry.key == key
        ) {
            entry.age =
                frameCounter_;

            return &entry;
        }

        if (
            !entry.valid &&
            freeEntry == nullptr
        ) {
            freeEntry = &entry;
        }

        if (
            oldest == nullptr ||
            entry.age < oldest->age
        ) {
            oldest = &entry;
        }
    }

    OrientationEntry* entry =
        freeEntry != nullptr
            ? freeEntry
            : oldest;

    if (entry == nullptr) {
        return nullptr;
    }

    *entry = {};

    entry->key = key;
    entry->age = frameCounter_;

    return entry;
}

void ItemPhysics::expireOldEntries() noexcept {
    constexpr std::uint64_t
        kMaxAge = 600;

    for (auto& entry : orientationCache_) {
        if (!entry.valid) {
            continue;
        }

        if (
            frameCounter_ > entry.age &&
            frameCounter_ - entry.age >
                kMaxAge
        ) {
            entry = {};
        }
    }
}

} // namespace levi::modules
