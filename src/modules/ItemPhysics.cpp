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

        levi::core::Logger::error(
            "ItemPhysics: unsupported Minecraft version"
        );

        return false;
    }

    active_ = this;

    enabled_ = false;

    orientationCache_ = {};

    frameCounter_ = 0;

    /*
     * RenderItem owns the actual MatrixStack mutation.
     *
     * ItemPhysics only marks the world-item scope through
     * renderItemGroup.
     */
    levi::minecraft::ItemRenderer::
        setWorldTransformCallback(
            &ItemPhysics::
                worldTransformCallback
        );

    status_ =
        ModuleStatus::WaitingForTarget;

    levi::core::Logger::info(
        "ItemPhysics initialized"
    );

    return true;
}

void ItemPhysics::shutdown() noexcept {
    disable();

    groupHook_.remove();

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

    levi::core::Logger::info(
        "ItemPhysics shutdown"
    );
}

void ItemPhysics::tick(
    float deltaTime
) noexcept {
    (void)deltaTime;

    if (!enabled_) {
        return;
    }

    /*
     * ItemPhysics is intentionally render-driven.
     *
     * No MatrixStack mutation happens in tick().
     */
}

bool ItemPhysics::enable() noexcept {
    if (!groupHook_.installed()) {
        status_ =
            ModuleStatus::WaitingForTarget;

        levi::core::Logger::warning(
            "ItemPhysics: renderItemGroup hook unavailable"
        );

        return false;
    }

    enabled_ = true;

    levi::core::State::instance()
        .setItemPhysicsEnabled(
            true
        );

    status_ =
        ModuleStatus::Active;

    levi::core::Logger::info(
        "ItemPhysics enabled"
    );

    return true;
}

void ItemPhysics::disable() noexcept {
    enabled_ = false;

    levi::core::State::instance()
        .setItemPhysicsEnabled(
            false
        );

    if (
        status_ ==
        ModuleStatus::Active
    ) {
        status_ =
            groupHook_.installed()
                ? ModuleStatus::Ready
                : ModuleStatus::
                    WaitingForTarget;
    }

    levi::core::Logger::info(
        "ItemPhysics disabled"
    );
}

bool ItemPhysics::enabled()
    const noexcept {
    return enabled_;
}

ModuleStatus
ItemPhysics::status()
    const noexcept {
    return status_;
}

void ItemPhysics::bindNativeTarget(
    std::uintptr_t renderItemGroup
) noexcept {
    if (
        renderItemGroup == 0 ||
        groupHook_.installed()
    ) {
        return;
    }

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
            "ItemPhysics: failed to hook renderItemGroup"
        );

        status_ =
            ModuleStatus::WaitingForTarget;

        return;
    }

    status_ =
        ModuleStatus::Ready;

    levi::core::Logger::info(
        "ItemPhysics: renderItemGroup hook active"
    );
}

bool ItemPhysics::hookInstalled()
    const noexcept {
    return groupHook_.installed();
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
    auto* module =
        active_;

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

    if (module != nullptr) {
        ++module->frameCounter_;

        /*
         * Periodically expire world-item orientation
         * entries to avoid pointer reuse creating stale
         * transforms.
         */
        if (
            (
                module->frameCounter_ &
                0x7F
            ) == 0
        ) {
            module->expireOldEntries();
        }
    }

    /*
     * Critical:
     *
     * World items can enter RenderItem with the same
     * renderingMainHand flag as first-person items.
     *
     * This thread-local scope prevents ViewModel from
     * touching dropped items.
     */
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
    auto* module =
        active_;

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

    /*
     * Keep current model scale while replacing only the
     * continuously changing rotational basis.
     */
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

    /*
     * First time this world entity is seen:
     *
     * capture its own model basis.
     *
     * This is the important difference from the buggy
     * reference mod: no universal 90-degree correction is
     * applied.
     *
     * Sword, block, shield, banner, etc. preserve their
     * original model orientation.
     */
    if (!entry->valid) {
        bool valid = true;

        valid &=
            normalize3(
                matrix->m[0],
                matrix->m[1],
                matrix->m[2],
                entry->basis[0],
                entry->basis[1],
                entry->basis[2]
            );

        valid &=
            normalize3(
                matrix->m[4],
                matrix->m[5],
                matrix->m[6],
                entry->basis[3],
                entry->basis[4],
                entry->basis[5]
            );

        valid &=
            normalize3(
                matrix->m[8],
                matrix->m[9],
                matrix->m[10],
                entry->basis[6],
                entry->basis[7],
                entry->basis[8]
            );

        if (!valid) {
            return;
        }

        entry->valid = true;
    }

    /*
     * Replace vanilla time-dependent orientation with the
     * stable captured basis while preserving current scale.
     */

    matrix->m[0] =
        entry->basis[0] *
        scaleX;

    matrix->m[1] =
        entry->basis[1] *
        scaleX;

    matrix->m[2] =
        entry->basis[2] *
        scaleX;

    matrix->m[4] =
        entry->basis[3] *
        scaleY;

    matrix->m[5] =
        entry->basis[4] *
        scaleY;

    matrix->m[6] =
        entry->basis[5] *
        scaleY;

    matrix->m[8] =
        entry->basis[6] *
        scaleZ;

    matrix->m[9] =
        entry->basis[7] *
        scaleZ;

    matrix->m[10] =
        entry->basis[8] *
        scaleZ;

    matrixStack.markDirty();

    entry->age =
        frameCounter_;
}

ItemPhysics::OrientationEntry*
ItemPhysics::findOrCreateEntry(
    void* key
) noexcept {
    OrientationEntry* freeEntry =
        nullptr;

    OrientationEntry* oldestEntry =
        nullptr;

    for (
        auto& entry :
        orientationCache_
    ) {
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
            freeEntry =
                &entry;
        }

        if (
            oldestEntry == nullptr ||
            entry.age <
                oldestEntry->age
        ) {
            oldestEntry =
                &entry;
        }
    }

    OrientationEntry* target =
        freeEntry != nullptr
            ? freeEntry
            : oldestEntry;

    if (target == nullptr) {
        return nullptr;
    }

    *target = {};

    target->key = key;

    target->age =
        frameCounter_;

    return target;
}

void ItemPhysics::expireOldEntries()
    noexcept {
    constexpr std::uint64_t
        kMaximumAge = 600;

    for (
        auto& entry :
        orientationCache_
    ) {
        if (!entry.valid) {
            continue;
        }

        if (
            frameCounter_ >
                entry.age &&
            frameCounter_ -
                entry.age >
                kMaximumAge
        ) {
            entry = {};
        }
    }
}

} // namespace levi::modules
