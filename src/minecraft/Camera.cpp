#include "levi/minecraft/Camera.hpp"

#include "levi/core/Logger.hpp"

namespace levi::minecraft {

bool Camera::attach(
    std::uintptr_t getPerspective,
    std::uintptr_t applyTurnDelta
) noexcept {
    bool any = false;

    if (
        getPerspective != 0 &&
        !perspectiveHook_.installed()
    ) {
        perspectiveHook_ =
            levi::memory::Hook(
                getPerspective,
                reinterpret_cast<void*>(
                    &Camera::perspectiveDetour
                )
            );

        if (perspectiveHook_.install()) {
            any = true;

            core::Logger::info(
                "Camera: GetPerspective hook active"
            );
        }
    } else if (perspectiveHook_.installed()) {
        any = true;
    }

    if (
        applyTurnDelta != 0 &&
        !turnHook_.installed()
    ) {
        turnHook_ =
            levi::memory::Hook(
                applyTurnDelta,
                reinterpret_cast<void*>(
                    &Camera::turnDeltaDetour
                )
            );

        if (turnHook_.install()) {
            any = true;

            core::Logger::info(
                "Camera: ApplyTurnDelta hook active"
            );
        }
    } else if (turnHook_.installed()) {
        any = true;
    }

    return any;
}

void Camera::detach() noexcept {
    turnHook_.remove();
    perspectiveHook_.remove();

    perspectiveObserver_ = nullptr;
    perspectiveOverride_ = nullptr;
    turnHandler_ = nullptr;
}

bool Camera::perspectiveHooked() noexcept {
    return perspectiveHook_.installed();
}

bool Camera::turnHooked() noexcept {
    return turnHook_.installed();
}

void Camera::setPerspectiveObserver(
    PerspectiveObserver observer
) noexcept {
    perspectiveObserver_ = observer;
}

void Camera::setPerspectiveOverride(
    PerspectiveOverride callback
) noexcept {
    perspectiveOverride_ = callback;
}

void Camera::setTurnDeltaHandler(
    TurnDeltaHandler callback
) noexcept {
    turnHandler_ = callback;
}

int Camera::perspectiveDetour(
    void* self
) noexcept {
    const auto original =
        reinterpret_cast<GetPerspectiveFn>(
            perspectiveHook_.original()
        );

    int result =
        original != nullptr
            ? original(self)
            : 0;

    if (perspectiveObserver_ != nullptr) {
        perspectiveObserver_(
            result
        );
    }

    if (perspectiveOverride_ != nullptr) {
        result =
            perspectiveOverride_(
                result
            );
    }

    return result;
}

void Camera::turnDeltaDetour(
    void* player,
    TurnDelta* delta
) noexcept {
    const auto original =
        reinterpret_cast<ApplyTurnDeltaFn>(
            turnHook_.original()
        );

    if (original == nullptr) {
        return;
    }

    if (
        delta != nullptr &&
        turnHandler_ != nullptr &&
        turnHandler_(
            delta->x,
            delta->y
        )
    ) {
        TurnDelta zero{};

        original(
            player,
            &zero
        );

        return;
    }

    original(
        player,
        delta
    );
}

} // namespace levi::minecraft
