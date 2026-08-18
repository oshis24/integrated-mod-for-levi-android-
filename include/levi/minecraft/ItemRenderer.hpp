#pragma once

#include "levi/math/Transform.hpp"
#include "levi/minecraft/MatrixStack.hpp"
#include "levi/minecraft/RenderContext.hpp"

#include "levi/memory/Hook.hpp"

#include <cstdint>

namespace levi::minecraft {

class ItemRenderer final {
public:

    /*
     * This is the native function boundary resolved from
     * Minecraft 1.26.44.3.
     *
     * The exact ABI is intentionally kept identical to the
     * current repository boundary.
     */
    using RenderFirstPersonFn =
        void(*)(
            void* self,
            RenderContext* context,
            MatrixStack* matrixStack
        );

public:

    static bool attach(
        std::uintptr_t target
    ) noexcept;

    static bool detach() noexcept;

    static bool attached() noexcept;

    static std::uintptr_t target() noexcept;

    static void setViewModelEnabled(
        bool enabled
    ) noexcept;

    static bool viewModelEnabled() noexcept;

    static void setViewModelTransform(
        const levi::math::Transform& transform
    ) noexcept;

    static const levi::math::Transform&
    viewModelTransform() noexcept;

    static void* original() noexcept;

private:

    static void hookRenderFirstPerson(
        void* self,
        RenderContext* context,
        MatrixStack* matrixStack
    ) noexcept;

private:

    inline static levi::memory::Hook hook_{};

    inline static std::uintptr_t
        target_{0};

    inline static bool
        attached_{false};

    inline static bool
        viewModelEnabled_{false};

    inline static levi::math::Transform
        viewModelTransform_{};
};

} // namespace levi::minecraft
