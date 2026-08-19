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

        return false;
    }

    enabled_ = false;

    setupAndRenderTarget_ = 0;
    renderItemGroupTarget_ = 0;

    status_ =
        ModuleStatus::WaitingForTarget;

    core::Logger::info(
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
}

bool ItemPhysics::enable() noexcept {
    /*
     * We need at least one world-item render boundary.
     */
    if (
        !nativeTargetsResolved()
    ) {
        status_ =
            ModuleStatus::WaitingForTarget;

        return false;
    }

    enabled_ = true;

    core::State::instance()
        .setItemPhysicsEnabled(true);

    status_ =
        ModuleStatus::Active;

    core::Logger::info(
        "ItemPhysics enabled"
    );

    return true;
}

void ItemPhysics::disable() noexcept {
    enabled_ = false;

    core::State::instance()
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

    if (
        nativeTargetsResolved()
    ) {
        status_ =
            ModuleStatus::Ready;

        core::Logger::info(
            "ItemPhysics native boundary resolved"
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
    return
        setupAndRenderTarget_ != 0 ||
        renderItemGroupTarget_ != 0;
}

ItemPhysicsTransform
ItemPhysics::transformFor(
    ItemVisualType type
) const noexcept {
    ItemPhysicsTransform result;

    result.visualType =
        type;

    /*
     * The reference mod's major visual problem comes from
     * modifying orientation on top of vanilla continuous spin.
     *
     * We therefore treat "replaceVanillaSpin" as a property of
     * the final transform stage instead of adding another spin.
     */
    result.replaceVanillaSpin = true;

    switch (type) {

        case ItemVisualType::Shield:
            /*
             * Do not force the generic flat-item orientation.
             */
            result.transform.rotation = {
                0.0f,
                0.0f,
                0.0f
            };
            break;

        case ItemVisualType::Banner:
            /*
             * Same principle: preserve the model's own basis.
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
