#pragma once

#include "levi/math/Transform.hpp"
#include "levi/memory/Hook.hpp"
#include "levi/minecraft/MatrixStack.hpp"

#include <cstdint>

namespace levi::minecraft {

class ItemRenderer final {
public:
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

    using WorldTransformCallback =
        void(*)(
            void* worldItemKey,
            MatrixStack& matrixStack
        ) noexcept;

    static bool attach(
        std::uintptr_t target
    ) noexcept;

    static bool detach() noexcept;

    static bool attached() noexcept;

    static void setViewModelEnabled(
        bool enabled
    ) noexcept;

    static void setViewModelTransform(
        const levi::math::Transform& transform
    ) noexcept;

    static void setViewModelPerspective(
        bool thirdPerson,
        bool applyThirdPerson
    ) noexcept;

    static void setWorldTransformCallback(
        WorldTransformCallback callback
    ) noexcept;

    static void beginWorldItemRender(
        void* worldItemKey
    ) noexcept;

    static void endWorldItemRender() noexcept;

    static bool inWorldItemRender() noexcept;

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
    inline static levi::memory::Hook hook_{};

    inline static bool attached_{false};

    inline static bool viewModelEnabled_{false};

    inline static bool thirdPerson_{false};

    inline static bool applyThirdPerson_{false};

    inline static levi::math::Transform
        viewModelTransform_{};

    inline static WorldTransformCallback
        worldTransformCallback_{nullptr};

    inline static thread_local int
        worldItemDepth_{0};

    inline static thread_local void*
        worldItemKey_{nullptr};
};

} // namespace levi::minecraft
