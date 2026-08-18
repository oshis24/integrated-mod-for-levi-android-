#include "levi/memory/PatternScanner.hpp"

#include "levi/core/Logger.hpp"

#include <cstdio>
#include <cstring>

namespace levi::memory {

namespace {

bool parseMapsLine(
    const char* line,
    std::uintptr_t& start,
    std::uintptr_t& end,
    char permissions[5],
    char* path,
    std::size_t pathSize
) {
    if (line == nullptr) {
        return false;
    }

    unsigned long long startAddress = 0;
    unsigned long long endAddress = 0;

    permissions[0] = '\0';

    int consumed = 0;

    const int result = std::sscanf(
        line,
        "%llx-%llx %4s %*s %*s %*s %n",
        &startAddress,
        &endAddress,
        permissions,
        &consumed
    );

    if (result < 3) {
        return false;
    }

    start = static_cast<std::uintptr_t>(startAddress);
    end = static_cast<std::uintptr_t>(endAddress);

    if (path != nullptr && pathSize > 0) {
        path[0] = '\0';

        if (consumed > 0) {
            const char* remaining = line + consumed;

            while (*remaining == ' ') {
                ++remaining;
            }

            std::strncpy(
                path,
                remaining,
                pathSize - 1
            );

            path[pathSize - 1] = '\0';

            const std::size_t length = std::strlen(path);

            if (length > 0 && path[length - 1] == '\n') {
                path[length - 1] = '\0';
            }
        }
    }

    return true;
}

bool pathMatchesLibrary(
    const char* path,
    const char* libraryName
) {
    if (path == nullptr || libraryName == nullptr) {
        return false;
    }

    const std::size_t pathLength = std::strlen(path);
    const std::size_t nameLength = std::strlen(libraryName);

    if (nameLength > pathLength) {
        return false;
    }

    const char* suffix =
        path + (pathLength - nameLength);

    return std::strcmp(suffix, libraryName) == 0;
}

} // namespace

bool PatternScanner::findTextRange(
    const char* libraryName,
    MemoryRange& range
) noexcept {

    range = {};

    if (libraryName == nullptr) {
        return false;
    }

    FILE* maps = std::fopen(
        "/proc/self/maps",
        "r"
    );

    if (maps == nullptr) {
        return false;
    }

    char line[1024];

    while (std::fgets(line, sizeof(line), maps) != nullptr) {
        std::uintptr_t start = 0;
        std::uintptr_t end = 0;

        char permissions[5]{};
        char path[512]{};

        if (!parseMapsLine(
                line,
                start,
                end,
                permissions,
                path,
                sizeof(path))) {
            continue;
        }

        /*
         * We specifically want executable mappings.
         *
         * r-xp is the usual .text mapping for native
         * Android shared libraries.
         */
        if (permissions[0] != 'r' ||
            permissions[2] != 'x') {
            continue;
        }

        if (!pathMatchesLibrary(path, libraryName)) {
            continue;
        }

        range.start = start;
        range.end = end;

        std::fclose(maps);

        core::Logger::debug(
            "Found executable mapping for %s: %p - %p",
            libraryName,
            reinterpret_cast<void*>(range.start),
            reinterpret_cast<void*>(range.end)
        );

        return true;
    }

    std::fclose(maps);

    core::Logger::warning(
        "Executable mapping not found: %s",
        libraryName
    );

    return false;
}

std::uintptr_t PatternScanner::find(
    const MemoryRange& range,
    const Pattern& pattern
) noexcept {

    if (!range.valid() || !pattern.valid()) {
        return 0;
    }

    return findInRange(
        range.start,
        range.end,
        pattern
    );
}

std::uintptr_t PatternScanner::find(
    const char* libraryName,
    const Pattern& pattern
) noexcept {

    MemoryRange range;

    if (!findTextRange(libraryName, range)) {
        return 0;
    }

    return find(range, pattern);
}

std::uintptr_t PatternScanner::findInRange(
    std::uintptr_t start,
    std::uintptr_t end,
    const Pattern& pattern
) noexcept {

    if (start == 0 ||
        end <= start ||
        !pattern.valid()) {
        return 0;
    }

    const std::size_t patternSize = pattern.size();
    const std::size_t rangeSize =
        static_cast<std::size_t>(end - start);

    if (patternSize > rangeSize) {
        return 0;
    }

    const auto* memory =
        reinterpret_cast<const std::uint8_t*>(start);

    const std::size_t limit =
        rangeSize - patternSize;

    for (std::size_t offset = 0;
         offset <= limit;
         ++offset) {

        if (pattern.matches(memory + offset)) {
            return start + offset;
        }
    }

    return 0;
}

} // namespace levi::memory
