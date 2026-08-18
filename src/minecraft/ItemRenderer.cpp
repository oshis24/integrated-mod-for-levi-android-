#include "levi/minecraft/ItemRenderer.hpp"

#include "levi/core/Logger.hpp"
#include "levi/memory/VTable.hpp"

namespace levi::minecraft {

namespace {

/*
 * From the previously reconstructed ItemInHandRenderer
 * vtable path:
 *
 *     vtable + 0x18
 *
 * therefore:
 *
 *     0x18 / sizeof(void*) = 3
 */
constexpr std::size_t kRenderFirstPersonVTableIndex = 3;

} // namespace

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

    const bool replaced =
        levi::memory::VTable::replace(
            renderer,
            kRenderFirstPersonVTableIndex,
            reinterpret_cast<void*>(
                &ItemRenderer::hookRenderFirstPerson
            ),
            &original
        );

    if (!replaced || original == nullptr) {
        levi::core::Logger::error(
            "ItemRenderer: vtable hook failed"
        );

        return false;
    }

    renderer_ = renderer;
    originalRenderFirstPerson_ = original;
    attached_ = true;

    levi::core::Logger::info(
        "ItemRenderer: renderFirstPerson hook attached"
    );

    return true;
}

bool ItemRenderer::detach() noexcept {
    /*
     * VTable::replace() currently does not retain enough
     * information to restore the original table entry safely.
     *
     * Therefore we deliberately do not pretend that detach
     * is supported yet.
     *
     * Runtime shutdown must keep the native object alive until
     * the owning renderer is destroyed.
     */
    return !attached_;
}

bool ItemRenderer::attached() noexcept {
    return attached_;
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

const levi::math::Transform&
ItemRenderer::viewModelTransform() noexcept {
    return viewModelTransform_;
}

void* ItemRenderer::renderer() noexcept {
    return renderer_;
}

void ItemRenderer::applyViewModel(
    MatrixStack* matrixStack
) noexcept {
    if (
        !viewModelEnabled_ ||
        matrixStack == nullptr ||
        !matrixStack->valid()
    ) {
        return;
    }

    matrixStack->apply(
        viewModelTransform_
    );
}

void ItemRenderer::hookRenderFirstPerson(
    void* self,
    RenderContext* context,
    MatrixStack* matrixStack
) noexcept {
    if (
        originalRenderFirstPerson_ == nullptr
    ) {
        return;
    }

    /*
     * Important:
     *
     * The transform is isolated to this render invocation.
     *
     *     push
     *       ↓
     *     ViewModel transform
     *       ↓
     *     vanilla render
     *       ↓
     *     pop
     */
    if (
        viewModelEnabled_ &&
        matrixStack != nullptr &&
        matrixStack->valid()
    ) {
        matrixStack->push();

        applyViewModel(
            matrixStack
        );

        reinterpret_cast<
            RenderFirstPersonFn
        >(
            originalRenderFirstPerson_
        )(
            self,
            context,
            matrixStack
        );

        matrixStack->pop();

        return;
    }

    reinterpret_cast<
        RenderFirstPersonFn
    >(
        originalRenderFirstPerson_
    )(
        self,
        context,
        matrixStack
    );
}

} // namespace levi::minecraft
