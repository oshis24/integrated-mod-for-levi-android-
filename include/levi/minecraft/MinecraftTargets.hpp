#pragma once

#include <cstdint>

namespace levi::minecraft {

struct MinecraftTargets final {
    std::uintptr_t renderItem{0};
    std::uintptr_t renderObject{0};

    std::uintptr_t getFov{0};
    std::uintptr_t getPerspective{0};
    std::uintptr_t localPlayerApplyTurnDelta{0};

    std::uintptr_t setupAndRender{0};
    std::uintptr_t renderItemGroup{0};

    bool viewModelValid() const noexcept {
        return renderItem != 0;
    }

    bool viewModelSpecialObjectValid() const noexcept {
        return renderObject != 0;
    }

    bool cameraValid() const noexcept {
        return
            getPerspective != 0 &&
            localPlayerApplyTurnDelta != 0;
    }

    bool itemPhysicsValid() const noexcept {
        return renderItemGroup != 0;
    }

    bool any() const noexcept {
        return
            renderItem != 0 ||
            renderObject != 0 ||
            getFov != 0 ||
            getPerspective != 0 ||
            localPlayerApplyTurnDelta != 0 ||
            setupAndRender != 0 ||
            renderItemGroup != 0;
    }
};

} // namespace levi::minecraft
