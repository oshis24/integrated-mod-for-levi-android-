#include "levi/minecraft/MinecraftProfile.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"
#include "levi/memory/Pattern.hpp"

namespace levi::minecraft {

namespace {

/*
 * BedrockTools signatures for Minecraft 1.26.44.3
 * reference layer.
 *
 * These are function-start signatures, not hardcoded RVA.
 */

constexpr const char* kRenderItemPattern =
    "? ? ? FC "
    "? ? ? 6D "
    "? ? ? 6D "
    "? ? ? 6D "
    "? ? ? A9 "
    "? ? ? A9 "
    "? ? ? A9 "
    "? ? ? A9 "
    "? ? ? A9 "
    "? ? ? A9 "
    "? ? ? 91 "
    "? ? ? D1 "
    "58 D0 3B D5 "
    "? ? ? F9";

constexpr const char* kGetFovPattern =
    "? ? ? FC "
    "? ? ? 6D "
    "? ? ? A9 "
    "? ? ? F9 "
    "? ? ? A9 "
    "? ? ? 91 "
    "08 40 20 1E";

constexpr const char* kGetPerspectivePattern =
    "? ? ? A9 "
    "FD 03 00 91 "
    "? ? ? F9 "
    "? ? ? F9 "
    "? ? ? F9 "
    "00 01 3F D6 "
    "? ? ? F9 "
    "? ? ? F9 "
    "? ? ? A8 "
    "20 00 1F D6 "
    "? ? ? A9 "
    "FD 03 00 91";

/*
 * This intentionally remains unresolved until we have an
 * exact 1.26.44.3 renderObject pattern.
 *
 * Atlas proves the renderObject conceptual boundary exists,
 * but the repo should not hook a guessed function.
 */

} // namespace

bool MinecraftProfile::supported() noexcept {
    return
        core::Runtime::instance()
            .isSupportedMinecraftVersion();
}

bool MinecraftProfile::libraryLoaded() noexcept {
    memory::MemoryRange text;

    return
        memory::PatternScanner::
            findTextRange(
                kLibrary,
                text
            );
}

std::uintptr_t
MinecraftProfile::resolveRenderItem(
    const memory::MemoryRange& text
) noexcept {
    return
        memory::PatternScanner::find(
            text,
            memory::Pattern(
                kRenderItemPattern
            )
        );
}

std::uintptr_t
MinecraftProfile::resolveGetFov(
    const memory::MemoryRange& text
) noexcept {
    return
        memory::PatternScanner::find(
            text,
            memory::Pattern(
                kGetFovPattern
            )
        );
}

std::uintptr_t
MinecraftProfile::resolveGetPerspective(
    const memory::MemoryRange& text
) noexcept {
    return
        memory::PatternScanner::find(
            text,
            memory::Pattern(
                kGetPerspectivePattern
            )
        );
}

bool MinecraftProfile::resolve(
    MinecraftTargets& targets
) noexcept {
    targets = {};

    if (!supported()) {
        return false;
    }

    memory::MemoryRange text;

    if (
        !memory::PatternScanner::
            findTextRange(
                kLibrary,
                text
            )
    ) {
        core::Logger::warning(
            "MinecraftProfile: "
            "libminecraftpe.so not loaded"
        );

        return false;
    }

    targets.renderItem =
        resolveRenderItem(text);

    targets.getFov =
        resolveGetFov(text);

    targets.getPerspective =
        resolveGetPerspective(text);

    core::Logger::info(
        "MinecraftProfile %s:",
        kVersion
    );

    core::Logger::info(
        "  RenderItem       = %p",
        reinterpret_cast<void*>(
            targets.renderItem
        )
    );

    core::Logger::info(
        "  GetFov           = %p",
        reinterpret_cast<void*>(
            targets.getFov
        )
    );

    core::Logger::info(
        "  GetPerspective   = %p",
        reinterpret_cast<void*>(
            targets.getPerspective
        )
    );

    /*
     * Do not report renderObject/setupAndRender as valid
     * until their exact native signatures are incorporated.
     */
    return targets.any();
}

} // namespace levi::minecraft
