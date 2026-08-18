#pragma once

#include "levi/minecraft/MatrixStack.hpp"
#include "levi/minecraft/RenderContext.hpp"

#include <cstdint>

namespace levi::minecraft {

class ItemRenderer final {
public:
    /*
     * ARM64 ABI:
     *
     * x0 = ItemInHandRenderer*
     * x1 = RenderContext*
     * x2 = MatrixStack*
     *
     * The remaining arguments are intentionally kept opaque
     * until the native render boundary supplies them.
     */
    using RenderFirstPersonFn = void(*)(
        void*,
        RenderContext*,
        MatrixStack*
    );

public:
    static bool attach(
        void* renderer
    ) noexcept;

    static bool detach() noexcept;

    static bool attached() noexcept;

    static void applyViewModel(
        MatrixStack* matrixStack
    ) noexcept;

    static void setViewModelEnabled(
        bool enabled
    ) noexcept;

    static void setViewModelTransform(
        const levi::math::Transform& transform
    ) noexcept;

    static const levi::math::Transform&
    viewModelTransform() noexcept;

    static void* renderer() noexcept;

private:
    static void hookRenderFirstPerson(
        void* self,
        RenderContext* context,
        MatrixStack* matrixStack
    ) noexcept;

private:
    inline static void* renderer_{nullptr};

    inline static void* originalRenderFirstPerson_{
        nullptr
    };

    inline static bool attached_{false};

    inline static bool viewModelEnabled_{false};

    inline static levi::math::Transform
        viewModelTransform_{};
};

} // namespace levi::minecraft
