#include "levi/minecraft/Camera.hpp"

#include "levi/core/Logger.hpp"

#include <cstring>

namespace levi::minecraft {

namespace {

/*
 * BedrockTools World.hpp:
 *
 * Actor::mActorRotationComponent = 0x218
 *
 * First Vec2:
 *     current pitch/yaw
 *
 * Second Vec2:
 *     previous pitch/yaw
 */
constexpr std::uintptr_t
    kActorRotationComponentOffset =
        0x218;

struct RotationPair final {
    float pitch;
    float yaw;

    float previousPitch;
    float previousYaw;
};

} // namespace

bool Camera::attach(
    std::uintptr_t getPerspective,
    std::uintptr_t applyTurnDelta,
    std::uintptr_t clientInstanceUpdate,
    std::uintptr_t screenViewRender,
    std::uintptr_t getLocalPlayer
) noexcept {
    bool any = false;

    getLocalPlayerTarget_ =
        getLocalPlayer;

    if (
        getPerspective != 0 &&
        !perspectiveHook_.installed()
    ) {
        perspectiveHook_ =
            levi::memory::Hook(
                getPerspective,
                reinterpret_cast<void*>(
                    &Camera::
                        perspectiveDetour
                )
            );

        if (perspectiveHook_.install()) {
            core::Logger::info(
                "Camera: GetPerspective hooked"
            );

            any = true;
        }
    } else if (
        perspectiveHook_.installed()
    ) {
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
                    &Camera::
                        turnDeltaDetour
                )
            );

        if (turnHook_.install()) {
            core::Logger::info(
                "Camera: ApplyTurnDelta hooked"
            );

            any = true;
        }
    } else if (
        turnHook_.installed()
    ) {
        any = true;
    }

    if (
        clientInstanceUpdate != 0 &&
        !clientInstanceHook_.installed()
    ) {
        clientInstanceHook_ =
            levi::memory::Hook(
                clientInstanceUpdate,
                reinterpret_cast<void*>(
                    &Camera::
                        clientInstanceUpdateDetour
                )
            );

        if (clientInstanceHook_.install()) {
            core::Logger::info(
                "Camera: ClientInstanceUpdate hooked"
            );

            any = true;
        }
    } else if (
        clientInstanceHook_.installed()
    ) {
        any = true;
    }

    if (
        screenViewRender != 0 &&
        !screenRenderHook_.installed()
    ) {
        screenRenderHook_ =
            levi::memory::Hook(
                screenViewRender,
                reinterpret_cast<void*>(
                    &Camera::
                        screenViewRenderDetour
                )
            );

        if (screenRenderHook_.install()) {
            core::Logger::info(
                "Camera: ScreenView::render hooked"
            );

            any = true;
        }
    } else if (
        screenRenderHook_.installed()
    ) {
        any = true;
    }

    return any;
}

void Camera::detach() noexcept {
    screenRenderHook_.remove();
    clientInstanceHook_.remove();
    turnHook_.remove();
    perspectiveHook_.remove();

    clientInstance_.store(
        nullptr,
        std::memory_order_release
    );

    getLocalPlayerTarget_ = 0;

    perspectiveObserver_ = nullptr;
    perspectiveOverride_ = nullptr;
    turnHandler_ = nullptr;
    visualRotationProvider_ = nullptr;
}

bool Camera::perspectiveHooked()
    noexcept {
    return perspectiveHook_.installed();
}

bool Camera::turnHooked()
    noexcept {
    return turnHook_.installed();
}

bool Camera::visualHooked()
    noexcept {
    return
        screenRenderHook_.installed() &&
        clientInstanceHook_.installed() &&
        getLocalPlayerTarget_ != 0;
}

void Camera::setPerspectiveObserver(
    PerspectiveObserver observer
) noexcept {
    perspectiveObserver_ =
        observer;
}

void Camera::setPerspectiveOverride(
    PerspectiveOverride callback
) noexcept {
    perspectiveOverride_ =
        callback;
}

void Camera::setTurnDeltaHandler(
    TurnDeltaHandler callback
) noexcept {
    turnHandler_ =
        callback;
}

void Camera::setVisualRotationProvider(
    VisualRotationProvider callback
) noexcept {
    visualRotationProvider_ =
        callback;
}

void* Camera::clientInstance()
    noexcept {
    return clientInstance_.load(
        std::memory_order_acquire
    );
}

int Camera::perspectiveDetour(
    void* self
) noexcept {
    const auto original =
        reinterpret_cast<
            GetPerspectiveFn
        >(
            perspectiveHook_.original()
        );

    int result =
        original != nullptr
            ? original(self)
            : 0;

    if (
        perspectiveObserver_ !=
        nullptr
    ) {
        perspectiveObserver_(
            result
        );
    }

    if (
        perspectiveOverride_ !=
        nullptr
    ) {
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
        reinterpret_cast<
            ApplyTurnDeltaFn
        >(
            turnHook_.original()
        );

    if (original == nullptr) {
        return;
    }

    if (
        delta != nullptr &&
        turnHandler_ != nullptr
    ) {
        const bool consumed =
            turnHandler_(
                delta->x,
                delta->y
            );

        if (consumed) {
            /*
             * Keep body/player orientation unchanged.
             */
            TurnDelta zero{};

            original(
                player,
                &zero
            );

            return;
        }
    }

    original(
        player,
        delta
    );
}

void* Camera::clientInstanceUpdateDetour(
    void* clientInstance,
    bool running
) noexcept {
    if (clientInstance != nullptr) {
        clientInstance_.store(
            clientInstance,
            std::memory_order_release
        );
    }

    const auto original =
        reinterpret_cast<
            ClientInstanceUpdateFn
        >(
            clientInstanceHook_.original()
        );

    if (original == nullptr) {
        return nullptr;
    }

    return original(
        clientInstance,
        running
    );
}

void Camera::screenViewRenderDetour(
    void* x0,
    void* x1,
    void* x2,
    void* x3,
    void* x4,
    void* x5,
    void* x6,
    void* x7
) noexcept {
    const auto original =
        reinterpret_cast<
            ScreenViewRenderFn
        >(
            screenRenderHook_.original()
        );

    if (original == nullptr) {
        return;
    }

    bool overridden = false;

    RotationPair snapshot{};

    void* rotationComponent =
        nullptr;

    if (
        visualRotationProvider_ !=
            nullptr &&
        getLocalPlayerTarget_ != 0
    ) {
        void* instance =
            clientInstance();

        if (instance != nullptr) {
            const auto getLocalPlayer =
                reinterpret_cast<
                    ClientInstanceGetLocalPlayerFn
                >(
                    getLocalPlayerTarget_
                );

            void* player =
                getLocalPlayer(
                    instance
                );

            if (player != nullptr) {
                const auto address =
                    reinterpret_cast<
                        std::uintptr_t
                    >(
                        player
                    );

                rotationComponent =
                    *reinterpret_cast<
                        void**
                    >(
                        address +
                        kActorRotationComponentOffset
                    );

                if (
                    rotationComponent !=
                    nullptr
                ) {
                    std::memcpy(
                        &snapshot,
                        rotationComponent,
                        sizeof(snapshot)
                    );

                    float pitch =
                        snapshot.pitch;

                    float yaw =
                        snapshot.yaw;

                    if (
                        visualRotationProvider_(
                            snapshot.pitch,
                            snapshot.yaw,
                            pitch,
                            yaw
                        )
                    ) {
                        auto* rotation =
                            reinterpret_cast<
                                float*
                            >(
                                rotationComponent
                            );

                        rotation[0] = pitch;
                        rotation[1] = yaw;

                        /*
                         * Keep current and previous values
                         * identical during render so
                         * interpolation cannot pull the camera
                         * back toward the player direction.
                         */
                        rotation[2] = pitch;
                        rotation[3] = yaw;

                        overridden = true;
                    }
                }
            }
        }
    }

    original(
        x0,
        x1,
        x2,
        x3,
        x4,
        x5,
        x6,
        x7
    );

    /*
     * Critical:
     *
     * visual Freelook must not permanently change player
     * rotation.
     */
    if (
        overridden &&
        rotationComponent != nullptr
    ) {
        std::memcpy(
            rotationComponent,
            &snapshot,
            sizeof(snapshot)
        );
    }
}

} // namespace levi::minecraft
