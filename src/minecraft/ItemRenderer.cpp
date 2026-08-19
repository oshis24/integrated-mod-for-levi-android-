#include "levi/minecraft/ItemRenderer.hpp"

#include "levi/core/Logger.hpp"

namespace levi::minecraft {

bool ItemRenderer::attach(
    std::uintptr_t renderItemTarget
) noexcept {
    if (renderItemTarget == 0) {
        return false;
    }

    if (attached_) {
        return true;
    }

    renderItemHook_ =
        levi::memory::Hook(
            renderItemTarget,
            reinterpret_cast<void*>(
                &ItemRenderer::
                    renderItemDetour
            )
        );

    if (!renderItemHook_.install()) {
        core::Logger::error(
            "ItemRenderer: RenderItem hook failed"
        );

        return false;
    }

    attached_ = true;

    core::Logger::info(
        "ItemRenderer: RenderItem hook active"
    );

    return true;
}

bool ItemRenderer::attachRenderObject(
    std::uintptr_t renderObjectTarget
) noexcept {
    if (renderObjectTarget == 0) {
        return false;
    }

    if (
        renderObjectHook_.installed()
    ) {
        return true;
    }

    renderObjectHook_ =
        levi::memory::Hook(
            renderObjectTarget,
            reinterpret_cast<void*>(
                &ItemRenderer::
                    renderObjectDetour
            )
        );

    if (!renderObjectHook_.install()) {
        core::Logger::error(
            "ItemRenderer: RenderObject hook failed"
        );

        return false;
    }

    core::Logger::info(
        "ItemRenderer: RenderObject hook active"
    );

    return true;
}

bool ItemRenderer::detach() noexcept {
    bool success = true;

    /*
     * Remove nested/special hook first.
     */
    if (
        renderObjectHook_.installed() &&
        !renderObjectHook_.remove()
    ) {
        success = false;
    }

    if (
        renderItemHook_.installed() &&
        !renderItemHook_.remove()
    ) {
        success = false;
    }

    attached_ = false;

    worldItemDepth_ = 0;
    worldItemKey_ = nullptr;
    viewModelRenderDepth_ = 0;

    return success;
}

bool ItemRenderer::attached()
    noexcept {
    return
        attached_ &&
        renderItemHook_.installed();
}

bool ItemRenderer::renderObjectAttached()
    noexcept {
    return
        renderObjectHook_.installed();
}

void ItemRenderer::setViewModelEnabled(
    bool enabled
) noexcept {
    viewModelEnabled_ = enabled;
}

void ItemRenderer::setViewModelTransform(
    const levi::math::Transform&
        transform
) noexcept {
    viewModelTransform_ =
        transform;
}

void ItemRenderer::setViewModelPerspective(
    bool thirdPerson,
    bool applyThirdPerson
) noexcept {
    thirdPerson_ =
        thirdPerson;

    applyThirdPerson_ =
        applyThirdPerson;
}

void ItemRenderer::setWorldTransformCallback(
    WorldTransformCallback callback
) noexcept {
    worldTransformCallback_ =
        callback;
}

void ItemRenderer::beginWorldItemRender(
    void* worldItemKey
) noexcept {
    if (worldItemDepth_ == 0) {
        worldItemKey_ =
            worldItemKey;
    }

    ++worldItemDepth_;
}

void ItemRenderer::endWorldItemRender()
    noexcept {
    if (worldItemDepth_ <= 0) {
        worldItemDepth_ = 0;
        worldItemKey_ = nullptr;

        return;
    }

    --worldItemDepth_;

    if (worldItemDepth_ == 0) {
        worldItemKey_ = nullptr;
    }
}

bool ItemRenderer::inWorldItemRender()
    noexcept {
    return worldItemDepth_ > 0;
}

void ItemRenderer::renderItemDetour(
    void* self,
    void* renderContext,
    void* entity,
    void* item,
    int posAndRotSet,
    int itemFlags,
    int useMatrixAsIs,
    int renderingMainHand
) noexcept {
    const auto original =
        reinterpret_cast<
            RenderItemFn
        >(
            renderItemHook_.original()
        );

    if (original == nullptr) {
        return;
    }

    auto matrixStack =
        MatrixStack::
            fromRenderContext(
                renderContext
            );

    /*
     * ItemPhysics/world-item scope always wins over
     * ViewModel.
     *
     * RE proved dropped items can also arrive here with
     * renderingMainHand == 1.
     */
    if (
        inWorldItemRender() &&
        worldTransformCallback_ !=
            nullptr &&
        matrixStack.current() !=
            nullptr
    ) {
        Matrix4 snapshot{};

        const bool haveSnapshot =
            matrixStack.snapshot(
                snapshot
            );

        worldTransformCallback_(
            worldItemKey_,
            matrixStack
        );

        matrixStack.markDirty();

        original(
            self,
            renderContext,
            entity,
            item,
            posAndRotSet,
            itemFlags,
            useMatrixAsIs,
            renderingMainHand
        );

        if (haveSnapshot) {
            matrixStack.restore(
                snapshot
            );
        }

        return;
    }

    const bool allowViewModel =
        viewModelEnabled_ &&
        renderingMainHand != 0 &&
        (
            !thirdPerson_ ||
            applyThirdPerson_
        );

    if (
        allowViewModel &&
        matrixStack.current() !=
            nullptr
    ) {
        Matrix4 snapshot{};

        const bool haveSnapshot =
            matrixStack.snapshot(
                snapshot
            );

        /*
         * Apply ONCE.
         */
        matrixStack.apply(
            viewModelTransform_
        );

        matrixStack.markDirty();

        /*
         * Any nested renderObject call now knows that it
         * belongs to this ViewModel render invocation.
         */
        ++viewModelRenderDepth_;

        original(
            self,
            renderContext,
            entity,
            item,
            posAndRotSet,
            itemFlags,
            useMatrixAsIs,
            renderingMainHand
        );

        --viewModelRenderDepth_;

        if (haveSnapshot) {
            matrixStack.restore(
                snapshot
            );
        }

        return;
    }

    original(
        self,
        renderContext,
        entity,
        item,
        posAndRotSet,
        itemFlags,
        useMatrixAsIs,
        renderingMainHand
    );
}

void* ItemRenderer::renderObjectDetour(
    void* self,
    void* renderContext,
    void* object,
    void* itemOrModel,
    int flags
) noexcept {
    const auto original =
        reinterpret_cast<
            RenderObjectFn
        >(
            renderObjectHook_.original()
        );

    if (original == nullptr) {
        return nullptr;
    }

    /*
     * Do NOT apply the ViewModel transform again.
     *
     * Atlas doesn't do that either.
     *
     * This detour exists to make the special object path
     * notice that the matrix was modified by the parent
     * RenderItem call.
     */
    const bool inViewModelObjectPath =
        viewModelEnabled_ &&
        viewModelRenderDepth_ > 0 &&
        !inWorldItemRender();

    if (!inViewModelObjectPath) {
        return original(
            self,
            renderContext,
            object,
            itemOrModel,
            flags
        );
    }

    auto matrixStack =
        MatrixStack::
            fromRenderContext(
                renderContext
            );

    /*
     * Atlas's renderObject detour writes the dirty state
     * both before and after original execution.
     */
    matrixStack.markDirty();

    void* result =
        original(
            self,
            renderContext,
            object,
            itemOrModel,
            flags
        );

    matrixStack.markDirty();

    return result;
}

} // namespace levi::minecraft
