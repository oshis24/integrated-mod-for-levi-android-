#include "levi/minecraft/ItemRenderer.hpp"

#include "levi/core/Logger.hpp"

namespace levi::minecraft {

void ItemRenderer::bind(
    RenderFirstPersonFn firstPerson,
    RenderItemFn renderItem,
    RenderObjectFn renderObject
) noexcept {
    firstPersonFn_ = firstPerson;
    renderItemFn_ = renderItem;
    renderObjectFn_ = renderObject;

    levi::core::Logger::info(
        "ItemRenderer render bridge bound"
    );
}

bool ItemRenderer::bound() noexcept {
    return
        firstPersonFn_ != nullptr &&
        renderItemFn_ != nullptr &&
        renderObjectFn_ != nullptr;
}

ItemRenderer::RenderFirstPersonFn
ItemRenderer::originalFirstPerson() noexcept {
    return firstPersonFn_;
}

ItemRenderer::RenderItemFn
ItemRenderer::originalRenderItem() noexcept {
    return renderItemFn_;
}

ItemRenderer::RenderObjectFn
ItemRenderer::originalRenderObject() noexcept {
    return renderObjectFn_;
}

void ItemRenderer::renderFirstPerson(
    void* self,
    RenderContext* context,
    MatrixStack* matrixStack,
    float partialTick
) noexcept {
    if (firstPersonFn_ == nullptr) {
        return;
    }

    firstPersonFn_(
        self,
        context,
        matrixStack,
        partialTick
    );
}

void ItemRenderer::renderItem(
    void* self,
    RenderContext* context,
    MatrixStack* matrixStack,
    std::int32_t itemId,
    float partialTick
) noexcept {
    if (renderItemFn_ == nullptr) {
        return;
    }

    renderItemFn_(
        self,
        context,
        matrixStack,
        itemId,
        partialTick
    );
}

void ItemRenderer::renderObject(
    void* self,
    RenderContext* context,
    MatrixStack* matrixStack,
    std::int32_t objectId,
    float partialTick
) noexcept {
    if (renderObjectFn_ == nullptr) {
        return;
    }

    renderObjectFn_(
        self,
        context,
        matrixStack,
        objectId,
        partialTick
    );
}

} // namespace levi::minecraft
