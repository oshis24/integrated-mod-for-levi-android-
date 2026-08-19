#include "levi/modules/Freelook.hpp"

#include "levi/core/Runtime.hpp"
#include "levi/core/State.hpp"
#include "levi/minecraft/Camera.hpp"

#include <algorithm>

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

    active_ = this;

    levi::minecraft::Camera::
        setTurnDeltaHandler(
            &Freelook::turnDeltaHandler
        );

    levi::minecraft::Camera::
        setPerspectiveOverride(
            &Freelook::
                perspectiveOverride
        );

    status_ =
        ModuleStatus::WaitingForTarget;

    return true;
}

void Freelook::shutdown() noexcept {
    disable();

    if (active_ == this) {
        active_ = nullptr;
    }

    status_ =
        ModuleStatus::Disabled;
}

void Freelook::tick(
    float deltaTime
) noexcept {
    (void)deltaTime;

    if (
        status_ ==
            ModuleStatus::WaitingForTarget &&
        levi::minecraft::Camera::
            turnHooked()
    ) {
        status_ =
            ModuleStatus::Ready;
    }
}

bool Freelook::enable() noexcept {
    if (
        !levi::minecraft::Camera::
            turnHooked()
    ) {
        status_ =
            ModuleStatus::WaitingForTarget;

        return false;
    }

    enabled_ = true;

    resetRotation();

    levi::core::State::instance()
        .setFreelookEnabled(true);

    status_ =
        ModuleStatus::Active;

    return true;
}

void Freelook::disable() noexcept {
    enabled_ = false;

    resetRotation();

    levi::core::State::instance()
        .setFreelookEnabled(false);

    if (
        status_ == ModuleStatus::Active
    ) {
        status_ =
            levi::minecraft::Camera::
                turnHooked()
                ? ModuleStatus::Ready
                : ModuleStatus::
                    WaitingForTarget;
    }
}

bool Freelook::enabled() const noexcept {
    return enabled_;
}

ModuleStatus
Freelook::status() const noexcept {
    return status_;
}

void Freelook::resetRotation() noexcept {
    yaw_ = 0.0f;
    pitch_ = 0.0f;
}

bool Freelook::turnDeltaHandler(
    float x,
    float y
) noexcept {
    auto* module = active_;

    if (
        module == nullptr ||
        !module->enabled_
    ) {
        return false;
    }

    module->yaw_ +=
        x * module->sensitivity_;

    module->pitch_ +=
        y * module->sensitivity_;

    module->pitch_ =
        std::clamp(
            module->pitch_,
            -89.0f,
            89.0f
        );

    /*
     * Suppress player/body turn.
     *
     * The final camera-yaw/pitch visual injection is the
     * remaining Freelook RE target.
     */
    return true;
}

int Freelook::perspectiveOverride(
    int originalPerspective
) noexcept {
    auto* module = active_;

    if (module == nullptr) {
        return originalPerspective;
    }

    if (!module->enabled_) {
        module->lockedPerspective_ =
            originalPerspective;

        return originalPerspective;
    }

    return module->lockedPerspective_;
}

} // namespace levi::modules
