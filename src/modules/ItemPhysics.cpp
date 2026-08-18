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

    /*
     * Unlike the reference ItemPhysics implementation,
     * we don't modify every item identically.
     *
     * The renderer will classify the visual representation
     * first, then request the appropriate transform.
     */

    status_ =
        ModuleStatus::Ready;

    levi::core::Logger::info(
        "ItemPhysics initialized"
    );

    return true;
}

void ItemPhysics::shutdown() noexcept {
    disable();

    status_ =
        ModuleStatus::Disabled;
}

void ItemPhysics::tick(
    float deltaTime
) noexcept {
    (void)deltaTime;
}

bool ItemPhysics::enable() noexcept {
    if (
        status_ != ModuleStatus::Ready &&
        status_ != ModuleStatus::Active
    ) {
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

ModuleStatus ItemPhysics::status() const noexcept {
    return status_;
}

ItemPhysicsTransform
ItemPhysics::transformFor(
    ItemVisualType type
) const noexcept {
    ItemPhysicsTransform result;

    result.visualType = type;

    /*
     * The important part here is that we DON'T use a universal
     * 90-degree correction.
     *
     * The broken reference mod effectively does that for a
     * number of representations, which is why some items look
     * horizontal while others continue spinning.
     */

    switch (type) {
        case ItemVisualType::Shield:
            result.transform.rotation = {
                0.0f,
                0.0f,
                0.0f
            };
            break;

        case ItemVisualType::Banner:
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
