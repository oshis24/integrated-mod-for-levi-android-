#pragma once

#include "levi/memory/Hook.hpp"

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

    using PerspectiveObserver =
        void(*)(int perspective) noexcept;

    using PerspectiveOverride =
        int(*)(int originalPerspective) noexcept;

    using TurnDeltaHandler =
        bool(*)(
            float x,
            float y
        ) noexcept;

    static bool attach(
        std::uintptr_t getPerspective,
        std::uintptr_t applyTurnDelta
    ) noexcept;

    static void detach() noexcept;

    static bool perspectiveHooked() noexcept;
    static bool turnHooked() noexcept;

    static void setPerspectiveObserver(
        PerspectiveObserver observer
    ) noexcept;

    static void setPerspectiveOverride(
        PerspectiveOverride callback
    ) noexcept;

    static void setTurnDeltaHandler(
        TurnDeltaHandler callback
    ) noexcept;

private:
    static int perspectiveDetour(
        void* self
    ) noexcept;

    static void turnDeltaDetour(
        void* player,
        TurnDelta* delta
    ) noexcept;

private:
    inline static levi::memory::Hook
        perspectiveHook_{};

    inline static levi::memory::Hook
        turnHook_{};

    inline static PerspectiveObserver
        perspectiveObserver_{nullptr};

    inline static PerspectiveOverride
        perspectiveOverride_{nullptr};

    inline static TurnDeltaHandler
        turnHandler_{nullptr};
};

} // namespace levi::minecraft
