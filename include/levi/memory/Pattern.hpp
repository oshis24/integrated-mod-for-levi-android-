#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace levi::memory {

struct Pattern {
    const char* bytes;
    const char* mask;
    std::size_t size;
};

Pattern parsePattern(std::string_view pattern);

uintptr_t findPattern(
    uintptr_t start,
    std::size_t size,
    const Pattern& pattern
);

uintptr_t findPattern(
    uintptr_t start,
    std::size_t size,
    std::string_view pattern
);

} // namespace levi::memory
