#pragma once

#include <cstdint>

namespace levi::minecraft {

struct ViewModelTargets {
    uintptr_t renderItem = 0;
    uintptr_t getFov = 0;
    uintptr_t getPerspective = 0;
};

struct MatrixStackLayout {
    std::uintptr_t blocks = 0x50;
    std::uintptr_t start  = 0x68;
    std::uintptr_t size   = 0x70;
};

struct RenderContextLayout {
    std::uintptr_t matrixStackWrapper = 0x28;
};

struct MatrixStackWrapperLayout {
    std::uintptr_t matrixStack = 0x18;
};

struct MinecraftProfile {
    const char* version;

    ViewModelTargets viewModel;

    RenderContextLayout renderContext;
    MatrixStackWrapperLayout matrixStackWrapper;
    MatrixStackLayout matrixStack;

    bool resolved = false;
};

MinecraftProfile& profile();

bool resolveProfile(uintptr_t libBase);

} // namespace levi::minecraft
