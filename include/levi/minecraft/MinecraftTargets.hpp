#pragma once

#include <cstdint>

namespace levi::minecraft {

struct MinecraftTargets final {
    // ViewModel
    std::uintptr_t renderItem{0};
    std::uintptr_t renderObject{0};
    std::uintptr_t getFov{0};

    // Camera / Freelook
    std::uintptr_t getPerspective{0};
    std::uintptr_t localPlayerApplyTurnDelta{0};

    std::uintptr_t clientInstanceUpdate{0};
    std::uintptr_t clientInstanceGetLocalPlayer{0};
    std::uintptr_t screenViewRender{0};

    // ItemPhysics
    std::uintptr_t renderItemGroup{0};

    bool viewModelValid() const noexcept {
        return renderItem != 0;
    }

    bool viewModelSpecialObjectValid() const noexcept {
        return renderObject != 0;
    }

    bool freelookValid() const noexcept {
        return
            getPerspective != 0 &&
            localPlayerApplyTurnDelta != 0 &&
            clientInstanceUpdate != 0 &&
            clientInstanceGetLocalPlayer != 0 &&
            screenViewRender != 0;
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
            clientInstanceUpdate != 0 ||
            clientInstanceGetLocalPlayer != 0 ||
            screenViewRender != 0 ||
            renderItemGroup != 0;
    }
};

} // namespace levi::minecraft
