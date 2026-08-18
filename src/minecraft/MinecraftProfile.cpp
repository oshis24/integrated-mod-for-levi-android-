#include "levi/minecraft/MinecraftProfile.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"

namespace levi::minecraft {

bool MinecraftProfile::supported() noexcept {
    const auto& runtime =
        levi::core::Runtime::instance();

    return
        runtime.isInitialized() &&
        runtime.isSupportedMinecraftVersion();
}

std::uintptr_t MinecraftProfile::resolveTarget(
    const char* name
) noexcept {
    /*
     * Target resolution is deliberately centralized here.
     *
     * Do NOT scatter Minecraft 1.26.44.3 addresses throughout
     * individual modules.
     *
     * Once a target has a confirmed signature/RVA, this is the
     * only layer that should know how it is resolved.
     */

    if (!supported()) {
        return 0;
    }

    if (name == nullptr) {
        return 0;
    }

    levi::core::Logger::debug(
        "MinecraftProfile: resolving target %s",
        name
    );

    /*
     * No speculative signature is installed here.
     *
     * Returning 0 means "target unavailable", rather than
     * silently hooking the wrong function.
     */

    return 0;
}

bool MinecraftProfile::resolve(
    RenderTargets& render,
    CameraTargets& camera
) noexcept {
    render = {};
    camera = {};

    if (!supported()) {
        levi::core::Logger::warning(
            "MinecraftProfile: unsupported runtime"
        );

        return false;
    }

    render.renderFirstPerson.address =
        resolveTarget(
            "ItemInHandRenderer::renderFirstPerson"
        );

    render.renderItem.address =
        resolveTarget(
            "ItemInHandRenderer::renderItem"
        );

    render.renderObject.address =
        resolveTarget(
            "ItemInHandRenderer::renderObject"
        );

    camera.cameraVFunc.address =
        resolveTarget(
            "Camera::rotation"
        );

    const bool anyRenderTarget =
        render.renderFirstPerson.valid() ||
        render.renderItem.valid() ||
        render.renderObject.valid();

    const bool anyCameraTarget =
        camera.cameraVFunc.valid();

    levi::core::Logger::info(
        "MinecraftProfile %s: render=%s camera=%s",
        kVersion,
        anyRenderTarget ? "available" : "unresolved",
        anyCameraTarget ? "available" : "unresolved"
    );

    return anyRenderTarget || anyCameraTarget;
}

} // namespace levi::minecraft
