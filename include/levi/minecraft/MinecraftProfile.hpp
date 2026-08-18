#pragma once

#include "levi/memory/PatternScanner.hpp"
#include "levi/minecraft/MinecraftTargets.hpp"

#include <cstdint>

namespace levi::minecraft {

class MinecraftProfile final {
public:
    static constexpr const char* kLibrary =
        "libminecraftpe.so";

    static constexpr const char* kVersion =
        "1.26.44.3";

public:
    static bool resolve(
        MinecraftTargets& targets
    ) noexcept;

    static bool libraryLoaded() noexcept;

    static bool supported() noexcept;

private:
    static std::uintptr_t
    resolveSetupAndRender(
        const memory::MemoryRange& text
    ) noexcept;

    static std::uintptr_t
    resolveRenderItemGroup(
        const memory::MemoryRange& text
    ) noexcept;

    static std::uintptr_t
    resolveItemInHandRenderer(
        const memory::MemoryRange& text
    ) noexcept;
};

} // namespace levi::minecraft
