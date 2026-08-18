#include "levi/modules/ViewModel.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"

namespace levi::modules {

bool ViewModel::initialize() noexcept {
    if (status_ != ModuleStatus::Disabled &&
        status_ != ModuleStatus::Failed) {
        return true;
    }

    auto& runtime = levi::core::Runtime::instance();

    if (!runtime.isInitialized()) {
        status_ = ModuleStatus::WaitingForRuntime;

        levi::core::Logger::debug(
            "ViewModel: runtime is not initialized"
        );

        return false;
    }

    /*
     * IMPORTANT:
     *
     * No Minecraft native address is used here yet.
     *
     * The actual ViewModel implementation will eventually
     * hook the verified first-person rendering path from
     * the 1.26.44.3 profile.
     */

    status_ = ModuleStatus::WaitingForTarget;

    levi::core::Logger::info(
        "ViewModel initialized; waiting for native target"
    );

    return true;
}

void ViewModel::shutdown() noexcept {
    disable();

    status_ = ModuleStatus::Disabled;

    /*
     * Native hook removal will be performed here once the
     * verified rendering target is connected.
     */
}

void ViewModel::tick(float deltaTime) noexcept {
    (void)deltaTime;

    if (!enabled_) {
        return;
    }

    /*
     * Actual transformation:
     *
     * translation
     * rotation
     * scale
     *
     * will be applied to the first-person MatrixStack
     * after the 1.26.44.3 render path is bound.
     */
}

bool ViewModel::enable() noexcept {
    if (status_ == ModuleStatus::Failed) {
        return false;
    }

    if (status_ == ModuleStatus::WaitingForTarget) {
        /*
         * Do not activate a module without a verified target.
         */
        levi::core::Logger::warning(
            "ViewModel cannot be enabled: "
            "native target is not verified"
        );

        return false;
    }

    enabled_ = true;
    status_ = ModuleStatus::Active;

    return true;
}

void ViewModel::disable() noexcept {
    enabled_ = false;

    if (status_ == ModuleStatus::Active) {
        status_ = ModuleStatus::Ready;
    }
}

bool ViewModel::enabled() const noexcept {
    return enabled_;
}

ModuleStatus ViewModel::status() const noexcept {
    return status_;
}

} // namespace levi::modules
