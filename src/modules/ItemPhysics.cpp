#include "levi/modules/ItemPhysics.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"
#include "levi/core/State.hpp"

namespace levi::modules {

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

    enabled_ = false;

    setupAndRenderTarget_ = 0;
    renderItemGroupTarget_ = 0;

    status_ =
        ModuleStatus::Ready;

    levi::core::Logger::info(
        "ItemPhysics initialized"
    );

    return true;
}

void ItemPhysics::shutdown() noexcept {
    disable();

    setupAndRenderTarget_ = 0;
    renderItemGroupTarget_ = 0;

    status_ =
        ModuleStatus::Disabled;
}

void ItemPhysics::tick(
    float deltaTime
) noexcept {
    (void)deltaTime;

    if (!enabled_) {
        return;
    }

    /*
     * Native rendering is intentionally not performed from
     * tick().
     *
     * ItemPhysics must execute inside the actual item-render
     * boundary so its MatrixStack state belongs to the item
     * currently being rendered.
     */
}

bool ItemPhysics::enable() noexcept {
    if (
        status_ != ModuleStatus::Ready &&
        status_ != ModuleStatus::Active
    ) {
        return false;
    }

    if (!nativeTargetsResolved()) {
        status_ =
            ModuleStatus::WaitingForTarget;

        levi::core::Logger::warning(
            "ItemPhysics: native render targets "
            "are not resolved yet"
        );

        return false;
    }

    enabled_ = true;

    levi::core::State::instance()
        .setItemPhysicsEnabled(true);

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
        .setItemPhysicsEnabled(false);

    if (
        status_ == ModuleStatus::Active
    ) {
        status_ =
            ModuleStatus::Ready;
    }
}

bool ItemPhysics::enabled() const noexcept {
    return enabled_;
}

ModuleStatus
ItemPhysics::status() const noexcept {
    return status_;
}

void ItemPhysics::bindNativeTargets(
    std::uintptr_t setupAndRender,
    std::uintptr_t renderItemGroup
) noexcept {
    setupAndRenderTarget_ =
        setupAndRender;

    renderItemGroupTarget_ =
        renderItemGroup;

    if (nativeTargetsResolved()) {
        if (
            status_ ==
            ModuleStatus::WaitingForTarget
        ) {
            status_ =
                ModuleStatus::Ready;
        }

        levi::core::Logger::info(
            "ItemPhysics: native targets bound"
        );
    }
}

std::uintptr_t
ItemPhysics::setupAndRenderTarget()
    const noexcept {
    return setupAndRenderTarget_;
}

std::uintptr_t
ItemPhysics::renderItemGroupTarget()
    const noexcept {
    return renderItemGroupTarget_;
}

bool ItemPhysics::nativeTargetsResolved()
    const noexcept {
    /*
     * setupAndRender is the primary native boundary.
     *
     * renderItemGroup is an additional render path.
     *
     * We don't require both simultaneously because different
     * builds/reference paths may expose only one of them.
     */
    return
        setupAndRenderTarget_ != 0 ||
        renderItemGroupTarget_ != 0;
}

ItemPhysicsTransform
ItemPhysics::transformFor(
    ItemVisualType type
) const noexcept {
    ItemPhysicsTransform result;

    result.visualType = type;

    /*
     * This function describes the visual policy only.
     *
     * It does NOT claim that the native item has already
     * been transformed. Actual transformation must happen
     * inside the native rendering callback.
     */

    switch (type) {
        case ItemVisualType::Shield:
            /*
             * Shield must use its own orientation path.
             *
             * Do not apply the generic flat-item correction.
             */
            result.transform.rotation = {
                0.0f,
                0.0f,
                0.0f
            };
            break;

        case ItemVisualType::Banner:
            /*
             * Banner has its own model orientation and should
             * not inherit the generic dropped-item rotation.
             */
            result.transform.rotation = {
                0.0f,
                0.0f,
                0.0f
            };
            break;

        case ItemVisualType::BlockItem:
            result.transform.rotation = {
                0.0f,
                0.0f,
                0.0f
            };
            break;

        case ItemVisualType::Tool:
        case ItemVisualType::Weapon:
        case ItemVisualType::FlatItem:
        case ItemVisualType::Unknown:
            result.transform.rotation = {
                0.0f,
                0.0f,
                0.0f
            };
            break;
    }

    return result;
}

} // namespace levi::modules
