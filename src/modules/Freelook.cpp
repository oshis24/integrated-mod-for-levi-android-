#include "levi/modules/Freelook.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"
#include "levi/core/State.hpp"

namespace levi::modules {

bool Freelook::initialize() noexcept {
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

    /*
     * The working Freelook reference separates:
     *
     * Input
     *   ↓
     * Freelook state
     *   ↓
     * Camera
     *
     * It does not use eglSwapBuffers as the camera function.
     */
    status_ =
        ModuleStatus::WaitingForTarget;

    core::Logger::info(
        "Freelook initialized"
    );

    return true;
}

void Freelook::shutdown() noexcept {
    disable();

    status_ =
        ModuleStatus::Disabled;
}

void Freelook::tick(
    float deltaTime
) noexcept {
    (void)deltaTime;

    if (!enabled_) {
        return;
    }

    /*
     * Camera mutation belongs in the native camera callback.
     *
     * Do not mutate a native camera object here from a tick
     * without a verified object lifetime.
     */
}

bool Freelook::enable() noexcept {
    if (
        status_ != ModuleStatus::Ready
    ) {
        return false;
    }

    enabled_ = true;

    core::State::instance()
        .setFreelookEnabled(true);

    status_ =
        ModuleStatus::Active;

    return true;
}

void Freelook::disable() noexcept {
    enabled_ = false;

    core::State::instance()
        .setFreelookEnabled(false);

    if (
        status_ == ModuleStatus::Active
    ) {
        status_ =
            ModuleStatus::Ready;
    }
}

bool Freelook::enabled() const noexcept {
    return enabled_;
}

ModuleStatus
Freelook::status() const noexcept {
    return status_;
}

} // namespace levi::modules
