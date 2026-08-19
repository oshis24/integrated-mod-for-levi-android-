#include "levi/minecraft/MinecraftProfile.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"

#include "levi/memory/Pattern.hpp"
#include "levi/memory/PatternScanner.hpp"

namespace levi::minecraft {

namespace {

/*
 * RenderItem
 *
 * Verified from BedrockTools and current
 * Minecraft 1.26.44.3.
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

/*
 * Atlas-equivalent renderObject.
 *
 * Current Minecraft 1.26.44.3:
 *
 *      0xADDFE80
 *
 * The full 40-byte prefix was checked against .text and
 * occurs exactly once.
 *
 * ARM64 ABI:
 *
 * x0 = renderer/self
 * x1 = RenderContext
 * x2 = render object
 * x3 = item/model information
 * w4 = render flag
 *
 * Return value:
 * x0 = pointer-like result
 */
constexpr const char* kRenderObjectPattern =
    "FD 7B BA A9 "
    "FC 6F 01 A9 "
    "FA 67 02 A9 "
    "F8 5F 03 A9 "
    "F6 57 04 A9 "
    "F4 4F 05 A9 "
    "FD 03 00 91 "
    "FF 43 0E D1 "
    "5B D0 3B D5 "
    "F8 03 02 AA";

/*
 * ViewModel item FOV.
 */
constexpr const char* kGetFovPattern =
    "? ? ? FC "
    "? ? ? 6D "
    "? ? ? A9 "
    "? ? ? F9 "
    "? ? ? A9 "
    "? ? ? 91 "
    "08 40 20 1E";

/*
 * VanillaCameraAPI::GetPerspective
 *
 * Also confirmed by vtable +0x38 / index 7.
 */
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
 * LocalPlayer::applyTurnDelta
 *
 * Used by the Freelook input/body-rotation split.
 */
constexpr const char* kApplyTurnDeltaPattern =
    "? ? ? D1 "
    "? ? ? FD "
    "? ? ? A9 "
    "? ? ? A9 "
    "? ? ? A9 "
    "? ? ? A9 "
    "? ? ? 91 "
    "56 D0 3B D5 "
    "F3 03 00 AA "
    "F4 03 01 AA "
    "? ? ? F9 "
    "? ? ? F8 "
    "? ? ? F9 "
    "? ? ? F9";

/*
 * Reconstructed from the working ItemPhysic Levi module.
 */
constexpr const char* kSetupAndRenderPattern =
    "EC 0F 17 FC "
    "EB 2B 01 6D "
    "E9 23 02 6D "
    "FD 7B 03 A9 "
    "FC 6F 04 A9 "
    "FA 67 05 A9 "
    "F8 5F 06 A9 "
    "F6 57 07 A9 "
    "F4 4F 08 A9 "
    "FD C3 00 91 "
    "FF 03 09 D1 "
    "?? ?? ?? D5 "
    "FC 03 00 AA";

/*
 * Working ItemPhysic renderItemGroup boundary.
 */
constexpr const char* kRenderItemGroupPattern =
    "FF C3 02 D1 "
    "EC 13 00 FD "
    "EB 2B 03 6D "
    "E9 23 04 6D "
    "FD 7B 05 A9 "
    "FC 6F 06 A9 "
    "FA 67 07 A9 "
    "F8 5F 08 A9 "
    "F6 57 09 A9 "
    "F4 4F 0A A9 "
    "FD 43 01 91 "
    "F4 03 02 AA";

std::uintptr_t scan(
    const memory::MemoryRange& text,
    const char* pattern
) noexcept {
    if (pattern == nullptr) {
        return 0;
    }

    const memory::Pattern parsed(
        pattern
    );

    if (!parsed.valid()) {
        return 0;
    }

    return memory::PatternScanner::find(
        text,
        parsed
    );
}

} // namespace

bool MinecraftProfile::supported() noexcept {
    return
        core::Runtime::instance()
            .isSupportedMinecraftVersion();
}

bool MinecraftProfile::libraryLoaded() noexcept {
    memory::MemoryRange text{};

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
    return scan(
        text,
        kRenderItemPattern
    );
}

std::uintptr_t
MinecraftProfile::resolveRenderObject(
    const memory::MemoryRange& text
) noexcept {
    return scan(
        text,
        kRenderObjectPattern
    );
}

std::uintptr_t
MinecraftProfile::resolveGetFov(
    const memory::MemoryRange& text
) noexcept {
    return scan(
        text,
        kGetFovPattern
    );
}

std::uintptr_t
MinecraftProfile::resolveGetPerspective(
    const memory::MemoryRange& text
) noexcept {
    return scan(
        text,
        kGetPerspectivePattern
    );
}

std::uintptr_t
MinecraftProfile::resolveLocalPlayerApplyTurnDelta(
    const memory::MemoryRange& text
) noexcept {
    return scan(
        text,
        kApplyTurnDeltaPattern
    );
}

std::uintptr_t
MinecraftProfile::resolveSetupAndRender(
    const memory::MemoryRange& text
) noexcept {
    return scan(
        text,
        kSetupAndRenderPattern
    );
}

std::uintptr_t
MinecraftProfile::resolveRenderItemGroup(
    const memory::MemoryRange& text
) noexcept {
    return scan(
        text,
        kRenderItemGroupPattern
    );
}

bool MinecraftProfile::resolve(
    MinecraftTargets& targets
) noexcept {
    targets = {};

    if (!supported()) {
        return false;
    }

    memory::MemoryRange text{};

    if (
        !memory::PatternScanner::
            findTextRange(
                kLibrary,
                text
            )
    ) {
        return false;
    }

    targets.renderItem =
        resolveRenderItem(
            text
        );

    targets.renderObject =
        resolveRenderObject(
            text
        );

    targets.getFov =
        resolveGetFov(
            text
        );

    targets.getPerspective =
        resolveGetPerspective(
            text
        );

    targets.localPlayerApplyTurnDelta =
        resolveLocalPlayerApplyTurnDelta(
            text
        );

    targets.setupAndRender =
        resolveSetupAndRender(
            text
        );

    targets.renderItemGroup =
        resolveRenderItemGroup(
            text
        );

    core::Logger::info(
        "MinecraftProfile %s resolved",
        kVersion
    );

    core::Logger::info(
        "  RenderItem                = %p",
        reinterpret_cast<void*>(
            targets.renderItem
        )
    );

    core::Logger::info(
        "  RenderObject              = %p",
        reinterpret_cast<void*>(
            targets.renderObject
        )
    );

    core::Logger::info(
        "  GetFov                    = %p",
        reinterpret_cast<void*>(
            targets.getFov
        )
    );

    core::Logger::info(
        "  GetPerspective            = %p",
        reinterpret_cast<void*>(
            targets.getPerspective
        )
    );

    core::Logger::info(
        "  LocalPlayerApplyTurnDelta = %p",
        reinterpret_cast<void*>(
            targets.localPlayerApplyTurnDelta
        )
    );

    core::Logger::info(
        "  setupAndRender            = %p",
        reinterpret_cast<void*>(
            targets.setupAndRender
        )
    );

    core::Logger::info(
        "  renderItemGroup           = %p",
        reinterpret_cast<void*>(
            targets.renderItemGroup
        )
    );

    return targets.any();
}

} // namespace levi::minecraft
