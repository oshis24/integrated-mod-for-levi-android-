#include "levi/modules/Freelook.hpp"

#include "levi/core/Logger.hpp"
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
            &Freelook::
                turnDeltaHandler
        );

    levi::minecraft::Camera::
        setPerspectiveOverride(
            &Freelook::
                perspectiveOverride
        );

    levi::minecraft::Camera::
        setVisualRotationProvider(
            &Freelook::
                visualRotationProvider
        );

    status_ =
        ModuleStatus::WaitingForTarget;

    core::Logger::info(
        "Freelook initialized"
    );

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
            ModuleStatus::
                WaitingForTarget &&
        levi::minecraft::Camera::
            turnHooked() &&
        levi::minecraft::Camera::
            visualHooked()
    ) {
        status_ =
            ModuleStatus::Ready;

        core::Logger::info(
            "Freelook native targets ready"
        );
    }
}

bool Freelook::enable() noexcept {
    if (
        !levi::minecraft::Camera::
            turnHooked() ||
        !levi::minecraft::Camera::
            visualHooked()
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

    core::Logger::info(
        "Freelook enabled"
    );

    return true;
}

void Freelook::disable() noexcept {
    enabled_ = false;

    resetRotation();

    levi::core::State::instance()
        .setFreelookEnabled(false);

    if (
        status_ ==
            ModuleStatus::Active
    ) {
        status_ =
            levi::minecraft::Camera::
                visualHooked()
                ? ModuleStatus::Ready
                : ModuleStatus::
                    WaitingForTarget;
    }

    core::Logger::info(
        "Freelook disabled"
    );
}

bool Freelook::enabled()
    const noexcept {
    return enabled_;
}

ModuleStatus
Freelook::status()
    const noexcept {
    return status_;
}

void Freelook::resetRotation()
    noexcept {
    basePitch_ = 0.0f;
    baseYaw_ = 0.0f;

    pitchOffset_ = 0.0f;
    yawOffset_ = 0.0f;

    visualInitialized_ =
        false;
}

bool Freelook::turnDeltaHandler(
    float x,
    float y
) noexcept {
    auto* module =
        active_;

    if (
        module == nullptr ||
        !module->enabled_
    ) {
        return false;
    }

    /*
     * Working reference:
     *
     * first lookDelta float  -> yaw
     * second lookDelta float -> pitch
     */
    module->yawOffset_ +=
        x *
        module->sensitivity_;

    module->pitchOffset_ +=
        y *
        module->sensitivity_;

    /*
     * Prevent an unbounded offset before the next visual
     * render.
     */
    module->pitchOffset_ =
        std::clamp(
            module->pitchOffset_,
            -178.0f,
            178.0f
        );

    module->yawOffset_ =
        wrapYaw(
            module->yawOffset_
        );

    /*
     * Input consumed:
     * suppress normal player/body turn.
     */
    return true;
}

int Freelook::perspectiveOverride(
    int originalPerspective
) noexcept {
    auto* module =
        active_;

    if (module == nullptr) {
        return originalPerspective;
    }

    if (!module->enabled_) {
        module->lockedPerspective_ =
            originalPerspective;

        return originalPerspective;
    }

    /*
     * Same concept as the working Freelook reference:
     * perspective does not change while freelooking.
     */
    return module->
        lockedPerspective_;
}

bool Freelook::visualRotationProvider(
    float vanillaPitch,
    float vanillaYaw,
    float& outPitch,
    float& outYaw
) noexcept {
    auto* module =
        active_;

    if (
        module == nullptr ||
        !module->enabled_
    ) {
        return false;
    }

    if (
        !module->
            visualInitialized_
    ) {
        module->basePitch_ =
            vanillaPitch;

        module->baseYaw_ =
            vanillaYaw;

        module->
            visualInitialized_ =
                true;
    }

    outPitch =
        std::clamp(
            module->basePitch_ +
                module->pitchOffset_,
            -89.0f,
            89.0f
        );

    outYaw =
        wrapYaw(
            module->baseYaw_ +
                module->yawOffset_
        );

    return true;
}

float Freelook::wrapYaw(
    float value
) noexcept {
    while (value > 180.0f) {
        value -= 360.0f;
    }

    while (value < -180.0f) {
        value += 360.0f;
    }

    return value;
}

} // namespace levi::modules
