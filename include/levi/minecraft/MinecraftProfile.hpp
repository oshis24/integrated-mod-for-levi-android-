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
    };

    struct CameraTargets {
        Target cameraVFunc;
    };

    struct Resolution {
        RenderTargets render{};
        CameraTargets camera{};

        bool any() const noexcept {
            return
                render.renderFirstPerson.valid() ||
                render.renderItem.valid() ||
                render.renderObject.valid() ||
                camera.cameraVFunc.valid();
        }
    };

public:
    static bool resolve(
        Resolution& result
    ) noexcept;

    static bool supported() noexcept;

    static bool libraryLoaded() noexcept;

private:
    static std::uintptr_t resolveTarget(
        const char* name
    ) noexcept;
};

} // namespace levi::minecraft
