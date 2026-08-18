#pragma once

#include "levi/minecraft/MatrixStack.hpp"
#include "levi/minecraft/RenderContext.hpp"

#include <cstdint>

namespace levi::minecraft {

class ItemRenderer final {
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

    /*
     * Generic render boundary.
     *
     * The exact native prototype is deliberately NOT encoded
     * until 1.26.44.3 has been verified against the supplied
     * libminecraftpe.so.
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

private:
    std::uintptr_t address_{0};
};

} // namespace levi::minecraft
