#include "levi/minecraft/MinecraftProfile.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"
#include "levi/memory/Pattern.hpp"

namespace levi::minecraft {

namespace {

/*
 * Reference:
 *
 * libItemPhysic.so
 *   └── SV::setupAndRender
 *
 * 48-byte ARM64 signature reconstructed from the
 * pair-encoded signature stored in the reference binary.
 */
constexpr std::uint8_t kSetupAndRender[] = {
    0xec, 0x0f, 0x17, 0xfc,
    0xeb, 0x2b, 0x01, 0x6d,
    0xe9, 0x23, 0x02, 0x6d,
    0xfd, 0x7b, 0x03, 0xa9,
    0xfc, 0x6f, 0x04, 0xa9,
    0xfa, 0x67, 0x05, 0xa9,
    0xf8, 0x5f, 0x06, 0xa9,
    0xf6, 0x57, 0x07, 0xa9,
    0xf4, 0x4f, 0x08, 0xa9,
    0xfd, 0xc3, 0x00, 0x91,
    0xff, 0x03, 0x09, 0xd1,
    0xd5, 0xfc, 0x03, 0x00
};

constexpr std::uint8_t kRenderItemGroup[] = {
    0xff, 0xc3, 0x02, 0xd1,
    0xec, 0x13, 0x00, 0xfd,
    0xeb, 0x2b, 0x03, 0x6d,
    0xe9, 0x23, 0x04, 0x6d,
    0xfd, 0x7b, 0x05, 0xa9,
    0xfc, 0x6f, 0x06, 0xa9,
    0xfa, 0x67, 0x07, 0xa9,
    0xf8, 0x5f, 0x08, 0xa9,
    0xf6, 0x57, 0x09, 0xa9,
    0xf4, 0x4f, 0x0a, 0xa9,
    0xfd, 0x43, 0x01, 0x91,
    0xf4, 0x03, 0x02, 0xaa
};

/*
 * This is the call-site signature used by the reference
 * before decoding its BL instruction.
 */
constexpr std::uint8_t kItemInHandCallsite[] = {
    0x27, 0x00, 0x80, 0x52,
    0xe0, 0x03, 0x15, 0xaa
};

memory::Pattern makeExactPattern(
    const std::uint8_t* bytes,
    std::size_t size
) noexcept {
    static constexpr std::uint8_t exact[64] = {
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff
    };

    return memory::Pattern(
        bytes,
        exact,
        size
    );
}

} // namespace

bool MinecraftProfile::supported() noexcept {
    return
        core::Runtime::instance()
            .isSupportedMinecraftVersion();
}

bool MinecraftProfile::libraryLoaded() noexcept {
    memory::MemoryRange text;

    return memory::PatternScanner::
        findTextRange(
            kLibrary,
            text
        );
}

std::uintptr_t
MinecraftProfile::resolveSetupAndRender(
    const memory::MemoryRange& text
) noexcept {
    const auto pattern =
        makeExactPattern(
            kSetupAndRender,
            sizeof(kSetupAndRender)
        );

    return memory::PatternScanner::find(
        text,
        pattern
    );
}

std::uintptr_t
MinecraftProfile::resolveRenderItemGroup(
    const memory::MemoryRange& text
) noexcept {
    const auto pattern =
        makeExactPattern(
            kRenderItemGroup,
            sizeof(kRenderItemGroup)
        );

    return memory::PatternScanner::find(
        text,
        pattern
    );
}

std::uintptr_t
MinecraftProfile::resolveItemInHandRenderer(
    const memory::MemoryRange& text
) noexcept {
    const auto pattern =
        makeExactPattern(
            kItemInHandCallsite,
            sizeof(kItemInHandCallsite)
        );

    const auto callsite =
        memory::PatternScanner::find(
            text,
            pattern
        );

    if (callsite == 0) {
        return 0;
    }

    /*
     * Reference implementation:
     *
     *     LDR W3, [X0,#4]!
     *
     * then verifies:
     *
     *     bits[31:26] == 0x25
     *
     * which is AArch64 BL.
     */
    const auto* instruction =
        reinterpret_cast<
            const std::uint32_t*
        >(callsite + 4);

    const std::uint32_t bl =
        *instruction;

    if (
        ((bl >> 26) & 0x3f) != 0x25
    ) {
        core::Logger::warning(
            "ItemInHandRenderer callsite found, "
            "but following instruction is not BL"
        );

        return 0;
    }

    /*
     * BL immediate:
     *
     * imm26 << 2
     * sign-extended to 64 bit
     */
    const std::int64_t imm26 =
        static_cast<std::int64_t>(
            bl & 0x03ffffffu
        );

    std::int64_t offset =
        imm26 << 2;

    if (
        (imm26 & (1LL << 25)) != 0
    ) {
        offset |=
            static_cast<std::int64_t>(
                0xfffffffff0000000ULL
            );
    }

    return
        static_cast<std::uintptr_t>(
            static_cast<
                std::int64_t
            >(callsite + 4) + offset
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
        return false;
    }

    targets.setupAndRender =
        resolveSetupAndRender(text);

    targets.renderItemGroup =
        resolveRenderItemGroup(text);

    targets.itemInHandRenderer =
        resolveItemInHandRenderer(text);

    core::Logger::info(
        "MinecraftProfile %s",
        kVersion
    );

    core::Logger::info(
        "setupAndRender = %p",
        reinterpret_cast<void*>(
            targets.setupAndRender
        )
    );

    core::Logger::info(
        "renderItemGroup = %p",
        reinterpret_cast<void*>(
            targets.renderItemGroup
        )
    );

    core::Logger::info(
        "ItemInHandRenderer = %p",
        reinterpret_cast<void*>(
            targets.itemInHandRenderer
        )
    );

    return targets.anyRenderTarget();
}

} // namespace levi::minecraft
