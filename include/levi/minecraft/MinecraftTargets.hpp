#pragma once

#include <cstdint>

namespace levi::minecraft {

struct MinecraftTargets final {
    std::uintptr_t setupAndRender{0};

    std::uintptr_t renderItemGroup{0};

    std::uintptr_t itemInHandRenderer{0};

    std::uintptr_t renderFirstPerson{0};

    std::uintptr_t renderItem{0};

    std::uintptr_t renderObject{0};

    std::uintptr_t camera{0};

    bool setupValid() const noexcept {
        return setupAndRender != 0;
    }

    bool itemGroupValid() const noexcept {
        return renderItemGroup != 0;
    }

    bool itemRendererValid() const noexcept {
        return itemInHandRenderer != 0;
    }

    bool anyRenderTarget() const noexcept {
        return
            setupAndRender != 0 ||
            renderItemGroup != 0 ||
            itemInHandRenderer != 0;
    }
};

} // namespace levi::minecraft
