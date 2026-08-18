#include "levi/minecraft/ItemRenderer.hpp"

#include "levi/core/Logger.hpp"

namespace levi::minecraft {

bool ItemRenderer::attach(
    std::uintptr_t target
) noexcept {

    if (target == 0) {
        levi::core::Logger::error(
            "ItemRenderer: invalid render target"
        );

        return false;
    }

    if (attached_) {
        return target_ == target;
    }

    hook_ =
        levi::memory::Hook(
            target,
            reinterpret_cast<void*>(
                &ItemRenderer::hookRenderFirstPerson
            )
        );

    if (!hook_.install()) {
        levi::core::Logger::error(
            "ItemRenderer: ARM64 hook installation failed"
        );

        return false;
    }

    target_ = target;
    attached_ = true;

    levi::core::Logger::info(
        "ItemRenderer: native renderFirstPerson hook active "
        "target=%p",
        reinterpret_cast<void*>(target_)
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
    return attached_ &&
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

void* ItemRenderer::original() noexcept {
    return hook_.original();
}

void ItemRenderer::hookRenderFirstPerson(
    void* self,
    RenderContext* context,
    MatrixStack* matrixStack
) noexcept {

    const auto original =
        reinterpret_cast<RenderFirstPersonFn>(
            hook_.original()
        );

    if (original == nullptr) {
        return;
    }

    /*
     * Never touch the native stack if the MatrixStack
     * object was not supplied by Minecraft.
     */
    if (
        !viewModelEnabled_ ||
        matrixStack == nullptr ||
        !matrixStack->valid()
    ) {
        original(
            self,
            context,
            matrixStack
        );

        return;
    }

    /*
     * Critical invariant:
     *
     *       push
     *          ↓
     *      ViewModel
     *          ↓
     *      vanilla render
     *          ↓
     *        pop
     *
     * This prevents ViewModel from leaking its transform
     * into subsequent rendering.
     */
    matrixStack->push();

    matrixStack->apply(
        viewModelTransform_
    );

    original(
        self,
        context,
        matrixStack
    );

    matrixStack->pop();
}

} // namespace levi::minecraft
