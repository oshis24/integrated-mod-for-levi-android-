#pragma once

#include "levi/memory/Hook.hpp"

#include <atomic>
#include <cstdint>

namespace levi::minecraft {

struct TurnDelta final {
    float x{0.0f};
    float y{0.0f};
};

class Camera final {
public:
    using GetPerspectiveFn =
        int(*)(void* self);

    using ApplyTurnDeltaFn =
        void(*)(
            void* player,
            TurnDelta* delta
        );

    using ClientInstanceUpdateFn =
        void* (*)(
            void* clientInstance,
            bool running
        );

    using ClientInstanceGetLocalPlayerFn =
        void* (*)(
            void* clientInstance
        );

    using ScreenViewRenderFn =
        void(*)(
            void* x0,
            void* x1,
            void* x2,
            void* x3,
            void* x4,
            void* x5,
            void* x6,
            void* x7
        );

    using PerspectiveObserver =
        void(*)(
            int perspective
        ) noexcept;

    using PerspectiveOverride =
        int(*)(
            int originalPerspective
        ) noexcept;

    using TurnDeltaHandler =
        bool(*)(
            float x,
            float y
        ) noexcept;

    /*
     * Return true when the caller should temporarily
     * override the visual player/camera rotation.
     */
    using VisualRotationProvider =
        bool(*)(
            float vanillaPitch,
            float vanillaYaw,
            float& outPitch,
            float& outYaw
        ) noexcept;

public:
    static bool attach(
        std::uintptr_t getPerspective,
        std::uintptr_t applyTurnDelta,
        std::uintptr_t clientInstanceUpdate,
        std::uintptr_t screenViewRender,
        std::uintptr_t getLocalPlayer
    ) noexcept;

    static void detach() noexcept;

    static bool perspectiveHooked() noexcept;

    static bool turnHooked() noexcept;

    static bool visualHooked() noexcept;

    static void setPerspectiveObserver(
        PerspectiveObserver observer
    ) noexcept;

    static void setPerspectiveOverride(
        PerspectiveOverride callback
    ) noexcept;

    static void setTurnDeltaHandler(
        TurnDeltaHandler callback
    ) noexcept;

    static void setVisualRotationProvider(
        VisualRotationProvider callback
    ) noexcept;

    static void* clientInstance() noexcept;

private:
    static int perspectiveDetour(
        void* self
    ) noexcept;

    static void turnDeltaDetour(
        void* player,
        TurnDelta* delta
    ) noexcept;

    static void* clientInstanceUpdateDetour(
        void* clientInstance,
        bool running
    ) noexcept;

    static void screenViewRenderDetour(
        void* x0,
        void* x1,
        void* x2,
        void* x3,
        void* x4,
        void* x5,
        void* x6,
        void* x7
    ) noexcept;

private:
    inline static
        levi::memory::Hook
            perspectiveHook_{};

    inline static
        levi::memory::Hook
            turnHook_{};

    inline static
        levi::memory::Hook
            clientInstanceHook_{};

    inline static
        levi::memory::Hook
            screenRenderHook_{};

    inline static
        std::uintptr_t
            getLocalPlayerTarget_{0};

    inline static
        std::atomic<void*>
            clientInstance_{nullptr};

    inline static
        PerspectiveObserver
            perspectiveObserver_{nullptr};

    inline static
        PerspectiveOverride
            perspectiveOverride_{nullptr};

    inline static
        TurnDeltaHandler
            turnHandler_{nullptr};

    inline static
        VisualRotationProvider
            visualRotationProvider_{
                nullptr
            };
};

} // namespace levi::minecraft
