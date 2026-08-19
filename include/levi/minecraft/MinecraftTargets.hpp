#pragma once

#include <cstdint>

namespace levi::minecraft {

struct MinecraftTargets final {
    std::uintptr_t renderItem{0};
    std::uintptr_t getFov{0};
    std::uintptr_t getPerspective{0};

    /*
     * World/special rendering.
     *
     * Kept separate from RenderItem because Atlas has a
     * distinct renderObject path and ItemPhysic has separate
     * setupAndRender/renderItemGroup paths.
     */
    std::uintptr_t renderObject{0};

    std::uintptr_t setupAndRender{0};
    std::uintptr_t renderItemGroup{0};

    bool renderItemValid() const noexcept {
        return renderItem != 0;
    }

    bool cameraValid() const noexcept {
        return
            getFov != 0 ||
            getPerspective != 0;
    }

    bool itemPhysicsValid() const noexcept {
        return
            setupAndRender != 0 ||
            renderItemGroup != 0;
    }

    bool any() const noexcept {
        return
            renderItemValid() ||
            cameraValid() ||
            itemPhysicsValid() ||
            renderObject != 0;
    }
};

} // namespace levi::minecraft
