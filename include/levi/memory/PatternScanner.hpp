#pragma once

#include "levi/memory/Pattern.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace levi::memory {

struct MemoryRange {
    std::uintptr_t start{0};
    std::uintptr_t end{0};

    std::size_t size() const noexcept {
        if (end <= start) {
            return 0;
        }

        return static_cast<std::size_t>(
            end - start
        );
    }

    bool valid() const noexcept {
        return start != 0 && end > start;
    }
};

class PatternScanner final {
public:
    /*
     * Find the executable (.text) range of a loaded library.
     */
    static bool findTextRange(
        const char* libraryName,
        MemoryRange& range
    ) noexcept;

    /*
     * Search a pattern inside a memory range.
     *
     * Returns 0 when no match is found.
     */
    static std::uintptr_t find(
        const MemoryRange& range,
        const Pattern& pattern
    ) noexcept;

    /*
     * Convenience function:
     *
     * library -> executable range -> pattern scan
     */
    static std::uintptr_t find(
        const char* libraryName,
        const Pattern& pattern
    ) noexcept;

    /*
     * Optional bounded search.
     *
     * Useful later when we have a known region around
     * a previously identified function.
     */
    static std::uintptr_t findInRange(
        std::uintptr_t start,
        std::uintptr_t end,
        const Pattern& pattern
    ) noexcept;
};

} // namespace levi::memory
