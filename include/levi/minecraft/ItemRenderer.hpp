#pragma once

#include "levi/math/Transform.hpp"
#include "levi/memory/Hook.hpp"
#include "levi/minecraft/RenderContext.hpp"

#include <cstdint>

namespace levi::minecraft {

class ItemRenderer final {
public:
    /*
     * Confirmed BedrockTools RenderItem ABI:
     *
     * x0 = this
     * x1 = RenderContext
     * x2 = entity
     * x3 = item
     * w4 = posAndRotSet
     * w5 = itemFlags
     * w6 = useMatrixAsIs
     * w7 = renderingMainHand
     */
    using RenderItemFn = void(*)(
        void* self,
        void* renderContext,
        void* entity,
        void* item,
        int posAndRotSet,
        int itemFlags,
        int useMatrixAsIs,
        int renderingMainHand
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

    static void setViewModelTransform(
        const levi::math::Transform& transform
    ) noexcept;

    static bool viewModelEnabled() noexcept;

    static void* original() noexcept;

private:
    static void renderItemDetour(
        void* self,
        void* renderContext,
        void* entity,
        void* item,
        int posAndRotSet,
        int itemFlags,
        int useMatrixAsIs,
        int renderingMainHand
    ) noexcept;

private:
    inline static memory::Hook hook_{};

    inline static std::uintptr_t
        target_{0};

    inline static bool
        attached_{false};

    inline static bool
        viewModelEnabled_{false};

    inline static levi::math::Transform
        transform_{};
};

} // namespace levi::minecraft
