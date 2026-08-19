#include "levi/minecraft/ItemRenderer.hpp"

#include "levi/core/Logger.hpp"
#include "levi/minecraft/MatrixStack.hpp"

namespace levi::minecraft {

namespace {

MatrixStack resolveMatrixStack(
    void* renderContext
) noexcept {
    /*
     * BedrockTools Render layout:
     *
     * RenderContext
     *     +0x28 -> MatrixStackWrapper
     *
     * MatrixStackWrapper
     *     +0x18 -> MatrixStack
     *
     * We only use this when the pointers are non-null.
     */
    if (renderContext == nullptr) {
        return {};
    }

    const auto context =
        reinterpret_cast<
            std::uintptr_t
        >(renderContext);

    const auto wrapper =
        *reinterpret_cast<
            std::uintptr_t*
        >(context + 0x28);

    if (wrapper == 0) {
        return {};
    }

    const auto stack =
        *reinterpret_cast<
            std::uintptr_t*
        >(wrapper + 0x18);

    return MatrixStack(stack);
}

} // namespace

bool ItemRenderer::attach(
    std::uintptr_t target
) noexcept {
    if (target == 0) {
        return false;
    }

    if (attached_) {
        return target_ == target;
    }

    hook_ =
        memory::Hook(
            target,
            reinterpret_cast<void*>(
                &ItemRenderer::renderItemDetour
            )
        );

    if (!hook_.install()) {
        core::Logger::error(
            "ItemRenderer: RenderItem hook failed"
        );

        return false;
    }

    target_ = target;
    attached_ = true;

    core::Logger::info(
        "ItemRenderer: RenderItem hook installed %p",
        reinterpret_cast<void*>(
            target_
        )
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

    target_ = 0;
    attached_ = false;

    return true;
}

bool ItemRenderer::attached() noexcept {
    return
        attached_ &&
        hook_.installed();
}

std::uintptr_t
ItemRenderer::target() noexcept {
    return target_;
}

void ItemRenderer::setViewModelEnabled(
    bool enabled
) noexcept {
    viewModelEnabled_ = enabled;
}

void ItemRenderer::setViewModelTransform(
    const levi::math::Transform& transform
) noexcept {
    transform_ = transform;
}

bool ItemRenderer::viewModelEnabled() noexcept {
    return viewModelEnabled_;
}

void* ItemRenderer::original() noexcept {
    return hook_.original();
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

    /*
     * We only transform first-person main-hand rendering.
     *
     * Off-hand / world rendering remains untouched here.
     */
    if (
        !viewModelEnabled_ ||
        renderingMainHand == 0
    ) {
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

        return;
    }

    auto matrixStack =
        resolveMatrixStack(
            renderContext
        );

    if (!matrixStack.valid()) {
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

        return;
    }

    /*
     * Main ViewModel implementation:
     *
     *     push
     *        |
     *        +-- translation
     *        +-- rotation
     *        +-- scale
     *        |
     *     original RenderItem
     *        |
     *      pop
     */
    matrixStack.push();

    matrixStack.apply(
        transform_
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

    matrixStack.pop();
}

} // namespace levi::minecraft
