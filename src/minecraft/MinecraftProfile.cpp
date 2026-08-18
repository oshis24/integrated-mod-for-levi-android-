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

bool MinecraftProfile::libraryLoaded() noexcept {
    levi::memory::MemoryRange range;

    return levi::memory::PatternScanner::findTextRange(
        kLibrary,
        range
    );
}

std::uintptr_t
MinecraftProfile::resolveTarget(
    const char* name
) noexcept {
    if (
        !supported() ||
        name == nullptr
    ) {
        return 0;
    }

    /*
     * IMPORTANT:
     *
     * Do not put guessed byte signatures here.
     *
     * The latest repository does not contain the actual
     * 1.26.44.3 instruction signatures, so returning zero is
     * safer than hooking a false positive.
     */
    levi::core::Logger::debug(
        "MinecraftProfile: target unresolved: %s",
        name
    );

    return 0;
}

bool MinecraftProfile::resolve(
    Resolution& result
) noexcept {
    result = {};

    if (!supported()) {
        return false;
    }

    if (!libraryLoaded()) {
        levi::core::Logger::warning(
            "MinecraftProfile: %s is not loaded",
            kLibrary
        );

        return false;
    }

    result.render.renderFirstPerson.address =
        resolveTarget(
            "ItemInHandRenderer::renderFirstPerson"
        );

    result.render.renderItem.address =
        resolveTarget(
            "ItemInHandRenderer::renderItem"
        );

    result.render.renderObject.address =
        resolveTarget(
            "ItemInHandRenderer::renderObject"
        );

    result.camera.cameraVFunc.address =
        resolveTarget(
            "Camera::rotation"
        );

    levi::core::Logger::info(
        "MinecraftProfile %s: resolution complete",
        kVersion
    );

    return result.any();
}

} // namespace levi::minecraft
