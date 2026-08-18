#include "levi/minecraft/ItemRenderer.hpp"

#include "levi/core/Logger.hpp"
#include "levi/memory/VTable.hpp"

namespace levi::minecraft {

bool ItemRenderer::attach(
    void* renderer
) noexcept {
    if (renderer == nullptr) {
        return false;
    }

    if (attached_) {
        return renderer_ == renderer;
    }

    void* original = nullptr;

    const bool success =
        levi::memory::VTable::replace(
            renderer,
            kRenderFirstPersonIndex,
            reinterpret_cast<void*>(
                &ItemRenderer::hookRenderFirstPerson
            ),
            &original
        );

    if (!success || original == nullptr) {
        levi::core::Logger::error(
            "ItemRenderer: failed to hook "
            "vtable[%zu]",
            kRenderFirstPersonIndex
        );

        return false;
    }

    renderer_ = renderer;
    originalRenderFirstPerson_ = original;
    attached_ = true;

    levi::core::Logger::info(
        "ItemRenderer: attached at vtable[%zu]",
        kRenderFirstPersonIndex
    );

    return true;
}

bool ItemRenderer::detach() noexcept {
    if (!attached_) {
        return true;
    }

    if (
        renderer_ == nullptr ||
        originalRenderFirstPerson_ == nullptr
    ) {
        attached_ = false;
        renderer_ = nullptr;
        originalRenderFirstPerson_ = nullptr;

        return true;
    }

    const bool restored =
        levi::memory::VTable::restore(
            renderer_,
            kRenderFirstPersonIndex,
            originalRenderFirstPerson_
        );

    if (!restored) {
        return false;
    }

    renderer_ = nullptr;
    originalRenderFirstPerson_ = nullptr;
    attached_ = false;

    return true;
}

bool ItemRenderer::attached() noexcept {
    return attached_;
}

void* ItemRenderer::renderer() noexcept {
    return renderer_;
}

void ItemRenderer::setViewModelEnabled(
    bool enabled
) noexcept {
    viewModelEnabled_ = enabled;
}

bool ItemRenderer::viewModelEnabled() noexcept {
    return viewModelEnabled_;
}

void ItemRenderer::setViewModelTransform(
    const levi::math::Transform& transform
) noexcept {
    viewModelTransform_ = transform;
}

const levi::math::Transform&
ItemRenderer::viewModelTransform() noexcept {
    return viewModelTransform_;
}

void ItemRenderer::hookRenderFirstPerson(
    void* self,
    RenderContext* context,
    MatrixStack* matrixStack
) noexcept {
    const auto original =
        originalRenderFirstPerson_;

    if (original == nullptr) {
        return;
    }

    const auto fn =
        reinterpret_cast<RenderFirstPersonFn>(
            original
        );

    /*
     * No MatrixStack => absolutely no modification.
     */
    if (
        !viewModelEnabled_ ||
        matrixStack == nullptr ||
        !matrixStack->valid()
    ) {
        fn(
            self,
            context,
            matrixStack
        );

        return;
    }

    matrixStack->push();

    matrixStack->apply(
        viewModelTransform_
    );

    fn(
        self,
        context,
        matrixStack
    );

    matrixStack->pop();
}

} // namespace levi::minecraft
