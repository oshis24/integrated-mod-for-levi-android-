#include "levi/minecraft/ItemRenderer.hpp"

#include "levi/core/Logger.hpp"

namespace levi::minecraft {

bool ItemRenderer::attach(
    std::uintptr_t target
) noexcept {
    if (target == 0) {
        return false;
    }

    if (attached_) {
        return true;
    }

    hook_ =
        levi::memory::Hook(
            target,
            reinterpret_cast<void*>(
                &ItemRenderer::renderItemDetour
            )
        );

    if (!hook_.install()) {
        core::Logger::error(
            "ItemRenderer: failed to hook RenderItem"
        );

        return false;
    }

    attached_ = true;

    core::Logger::info(
        "ItemRenderer: RenderItem hook active"
    );

    return true;
}

bool ItemRenderer::detach() noexcept {
    if (!attached_) {
        return true;
    }

    if (!hook_.remove()) {
        return false;
    }

    attached_ = false;

    return true;
}

bool ItemRenderer::attached() noexcept {
    return
        attached_ &&
        hook_.installed();
}

void ItemRenderer::setViewModelEnabled(
    bool enabled
) noexcept {
    viewModelEnabled_ = enabled;
}

void ItemRenderer::setViewModelTransform(
    const levi::math::Transform& transform
) noexcept {
    viewModelTransform_ = transform;
}

void ItemRenderer::setViewModelPerspective(
    bool thirdPerson,
    bool applyThirdPerson
) noexcept {
    thirdPerson_ = thirdPerson;
    applyThirdPerson_ = applyThirdPerson;
}

void ItemRenderer::setWorldTransformCallback(
    WorldTransformCallback callback
) noexcept {
    worldTransformCallback_ = callback;
}

void ItemRenderer::beginWorldItemRender(
    void* worldItemKey
) noexcept {
    if (worldItemDepth_ == 0) {
        worldItemKey_ = worldItemKey;
    }

    ++worldItemDepth_;
}

void ItemRenderer::endWorldItemRender() noexcept {
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

bool ItemRenderer::inWorldItemRender() noexcept {
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
        reinterpret_cast<RenderItemFn>(
            hook_.original()
        );

    if (original == nullptr) {
        return;
    }

    auto matrixStack =
        MatrixStack::fromRenderContext(
            renderContext
        );

    /*
     * Dropped-world-item scope takes priority.
     *
     * RE showed world items can reach RenderItem with
     * renderingMainHand == 1 too.
     */
    if (
        inWorldItemRender() &&
        worldTransformCallback_ != nullptr &&
        matrixStack.current() != nullptr
    ) {
        Matrix4 snapshot{};

        const bool haveSnapshot =
            matrixStack.snapshot(snapshot);

        worldTransformCallback_(
            worldItemKey_,
            matrixStack
        );

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
            matrixStack.restore(snapshot);
        }

        return;
    }

    const bool allowViewModel =
        viewModelEnabled_ &&
        renderingMainHand != 0 &&
        (!thirdPerson_ || applyThirdPerson_);

    if (
        allowViewModel &&
        matrixStack.current() != nullptr
    ) {
        Matrix4 snapshot{};

        const bool haveSnapshot =
            matrixStack.snapshot(snapshot);

        matrixStack.apply(
            viewModelTransform_
        );

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
            matrixStack.restore(snapshot);
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

} // namespace levi::minecraft
