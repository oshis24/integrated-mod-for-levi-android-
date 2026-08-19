#pragma once

#include "levi/math/Transform.hpp"
#include "levi/memory/Hook.hpp"
#include "levi/minecraft/MatrixStack.hpp"

#include <cstdint>

namespace levi::minecraft {

class ItemRenderer final {
public:
    /*
     * Confirmed RenderItem ABI.
     */
    using RenderItemFn =
        void(*)(
            void* self,
            void* renderContext,
            void* entity,
            void* item,
            int posAndRotSet,
            int itemFlags,
            int useMatrixAsIs,
            int renderingMainHand
        );

    /*
     * Atlas-equivalent renderObject ABI.
     *
     * Confirmed from both current RenderItem callers and
     * Atlas callback register preservation:
     *
     * x0 x1 x2 x3 w4
     *
     * Return x0 is pointer-like and consumed by caller.
     */
    using RenderObjectFn =
        void* (*)(
            void* self,
            void* renderContext,
            void* object,
            void* itemOrModel,
            int flags
        );

    using WorldTransformCallback =
        void(*)(
            void* worldItemKey,
            MatrixStack& matrixStack
        ) noexcept;

    static bool attach(
        std::uintptr_t renderItemTarget
    ) noexcept;

    static bool attachRenderObject(
        std::uintptr_t renderObjectTarget
    ) noexcept;

    static bool detach() noexcept;

    static bool attached() noexcept;

    static bool renderObjectAttached()
        noexcept;

    static void setViewModelEnabled(
        bool enabled
    ) noexcept;

    static void setViewModelTransform(
        const levi::math::Transform&
            transform
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

    static void endWorldItemRender()
        noexcept;

    static bool inWorldItemRender()
        noexcept;

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

    static void* renderObjectDetour(
        void* self,
        void* renderContext,
        void* object,
        void* itemOrModel,
        int flags
    ) noexcept;

private:
    inline static
        levi::memory::Hook
            renderItemHook_{};

    inline static
        levi::memory::Hook
            renderObjectHook_{};

    inline static bool
        attached_{false};

    inline static bool
        viewModelEnabled_{false};

    inline static bool
        thirdPerson_{false};

    inline static bool
        applyThirdPerson_{false};

    inline static
        levi::math::Transform
            viewModelTransform_{};

    inline static
        WorldTransformCallback
            worldTransformCallback_{
                nullptr
            };

    inline static thread_local int
        worldItemDepth_{0};

    inline static thread_local void*
        worldItemKey_{nullptr};

    /*
     * Non-zero only while our ViewModel-modified
     * RenderItem is executing.
     *
     * Nested renderObject calls can therefore tell whether
     * they belong to first-person ViewModel rendering.
     */
    inline static thread_local int
        viewModelRenderDepth_{0};
};

} // namespace levi::minecraft
