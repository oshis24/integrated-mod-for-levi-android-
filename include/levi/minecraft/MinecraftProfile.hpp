#pragma once

#include "levi/memory/PatternScanner.hpp"

#include <cstdint>

namespace levi::minecraft {

class MinecraftProfile final {
public:
    static constexpr const char* kLibrary =
        "libminecraftpe.so";

    static constexpr const char* kVersion =
        "1.26.44.3";

    struct Target {
        std::uintptr_t address{0};

        bool valid() const noexcept {
            return address != 0;
        }
    };

    struct RenderTargets {
        Target renderFirstPerson;
        Target renderItem;
        Target renderObject;

        bool valid() const noexcept {
            return
                renderFirstPerson.valid() &&
                renderItem.valid() &&
                renderObject.valid();
        }
    };

    struct CameraTargets {
        Target cameraVFunc;
    };

public:
    static bool resolve(
        RenderTargets& render,
        CameraTargets& camera
    ) noexcept;

    static bool supported() noexcept;

private:
    static std::uintptr_t resolveTarget(
        const char* name
    ) noexcept;
};

} // namespace levi::minecraft
