#include "levi/modules/Freelook.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"

namespace levi::modules {

bool Freelook::initialize() noexcept {
    if (status_ != ModuleStatus::Disabled &&
        status_ != ModuleStatus::Failed) {
        return true;
    }

    auto& runtime = levi::core::Runtime::instance();

    if (!runtime.isInitialized()) {
        status_ = ModuleStatus::WaitingForRuntime;

        levi::core::Logger::debug(
            "Freelook: runtime is not initialized"
        );

        return false;
    }

    /*
     * The supplied working Freelook reference gives us the
     * behavioral direction, but we still keep the native
     * target abstract until the 1.26.44.3 profile is verified.
     */

    status_ = ModuleStatus::WaitingForTarget;

    levi::core::Logger::info(
        "Freelook initialized; waiting for native target"
    );

    return true;
}

void Freelook::shutdown() noexcept {
    disable();

    status_ = ModuleStatus::Disabled;
}

void Freelook::tick(float deltaTime) noexcept {
    (void)deltaTime;

    if (!enabled_) {
        return;
    }

    /*
     * Future flow:
     *
     * Input
     *   ↓
     * freelook state
     *   ↓
     * camera rotation
     *   ↓
     * render camera
     *
     * while preserving player body/yaw independently.
     */
}

bool Freelook::enable() noexcept {
    if (status_ == ModuleStatus::Failed) {
        return false;
    }

    if (status_ == ModuleStatus::WaitingForTarget) {
        levi::core::Logger::warning(
            "Freelook cannot be enabled: "
            "native target is not verified"
        );

        return false;
    }

    enabled_ = true;
    status_ = ModuleStatus::Active;

    return true;
}

void Freelook::disable() noexcept {
    enabled_ = false;

    if (status_ == ModuleStatus::Active) {
        status_ = ModuleStatus::Ready;
    }
}

bool Freelook::enabled() const noexcept {
    return enabled_;
}

ModuleStatus Freelook::status() const noexcept {
    return status_;
}

} // namespace levi::modules
