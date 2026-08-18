#include "levi/modules/ItemPhysics.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"

namespace levi::modules {

bool ItemPhysics::initialize() noexcept {
    if (status_ != ModuleStatus::Disabled &&
        status_ != ModuleStatus::Failed) {
        return true;
    }

    auto& runtime =
        levi::core::Runtime::instance();

    if (!runtime.isInitialized()) {
        status_ = ModuleStatus::WaitingForRuntime;

        levi::core::Logger::debug(
            "ItemPhysics: runtime is not initialized"
        );

        return false;
    }

    /*
     * The final implementation will not simply rotate every
     * dropped item by a fixed amount.
     *
     * The RE target must allow us to distinguish:
     *
     *   - item entity creation
     *   - item entity render
     *   - item stack/count
     *   - item/block visual representation
     *   - rotation state
     *
     * This is particularly important for:
     *
     *   - shields
     *   - banners
     *   - block items
     *   - tools
     *   - generated/3D item models
     *
     * so that the result does not inherit the bugs of the
     * reference ItemPhysics implementation.
     */

    status_ = ModuleStatus::WaitingForTarget;

    levi::core::Logger::info(
        "ItemPhysics initialized; "
        "waiting for native target"
    );

    return true;
}

void ItemPhysics::shutdown() noexcept {
    disable();

    status_ = ModuleStatus::Disabled;
}

void ItemPhysics::tick(float deltaTime) noexcept {
    (void)deltaTime;

    if (!enabled_) {
        return;
    }

    /*
     * Item physics should eventually be evaluated from the
     * actual entity/render state rather than from the frame
     * counter alone.
     *
     * Desired behavior:
     *
     *   dropped item
     *       ↓
     *   identify visual/model
     *       ↓
     *   preserve vanilla position/scale
     *       ↓
     *   replace unwanted spin
     *       ↓
     *   apply stable orientation
     */

    // Native implementation pending RE profile.
}

bool ItemPhysics::enable() noexcept {
    if (status_ == ModuleStatus::Failed) {
        return false;
    }

    if (status_ == ModuleStatus::WaitingForTarget) {
        levi::core::Logger::warning(
            "ItemPhysics cannot be enabled: "
            "native target is not verified"
        );

        return false;
    }

    enabled_ = true;
    status_ = ModuleStatus::Active;

    return true;
}

void ItemPhysics::disable() noexcept {
    enabled_ = false;

    if (status_ == ModuleStatus::Active) {
        status_ = ModuleStatus::Ready;
    }
}

bool ItemPhysics::enabled() const noexcept {
    return enabled_;
}

ModuleStatus ItemPhysics::status() const noexcept {
    return status_;
}

} // namespace levi::modules
