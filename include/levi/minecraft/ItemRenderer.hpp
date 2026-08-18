#pragma once

#include "levi/math/Transform.hpp"
#include "levi/minecraft/MatrixStack.hpp"
#include "levi/minecraft/RenderContext.hpp"

#include <cstdint>

namespace levi::minecraft {

class ItemRenderer final {
public:
    /*
     * The callback signatures are intentionally kept as the
     * render-boundary ABI used by the Levi bridge.
     *
     * The object itself is still opaque.
     */

    using RenderFirstPersonFn = void(*)(
        void* self,
        RenderContext* context,
        MatrixStack* matrixStack,
        float partialTick
    );

    using RenderItemFn = void(*)(
        void* self,
        RenderContext* context,
        MatrixStack* matrixStack,
        std::int32_t itemId,
        float partialTick
    );

    using RenderObjectFn = void(*)(
        void* self,
        RenderContext* context,
        MatrixStack* matrixStack,
        std::int32_t objectId,
        float partialTick
    );

public:
    ItemRenderer() = default;

    explicit ItemRenderer(
        std::uintptr_t address
    )
        : address_(address) {
    }

    bool valid() const noexcept {
        return address_ != 0;
    }

    std::uintptr_t address() const noexcept {
        return address_;
    }

    static void bind(
        RenderFirstPersonFn firstPerson,
        RenderItemFn renderItem,
        RenderObjectFn renderObject
    ) noexcept;

    static bool bound() noexcept;

    static RenderFirstPersonFn
    originalFirstPerson() noexcept;

    static RenderItemFn
    originalRenderItem() noexcept;

    static RenderObjectFn
    originalRenderObject() noexcept;

    /*
     * These functions are called by the module hooks.
     */
    static void renderFirstPerson(
        void* self,
        RenderContext* context,
        MatrixStack* matrixStack,
        float partialTick
    ) noexcept;

    static void renderItem(
        void* self,
        RenderContext* context,
        MatrixStack* matrixStack,
        std::int32_t itemId,
        float partialTick
    ) noexcept;

    static void renderObject(
        void* self,
        RenderContext* context,
        MatrixStack* matrixStack,
        std::int32_t objectId,
        float partialTick
    ) noexcept;

private:
    std::uintptr_t address_{0};

    inline static RenderFirstPersonFn
        firstPersonFn_{nullptr};

    inline static RenderItemFn
        renderItemFn_{nullptr};

    inline static RenderObjectFn
        renderObjectFn_{nullptr};
};

} // namespace levi::minecraft
