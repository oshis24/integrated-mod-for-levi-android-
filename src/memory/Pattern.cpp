#include "memory/Pattern.hpp"

#include <cctype>
#include <cstdlib>
#include <vector>

namespace levi::memory {

Pattern parsePattern(std::string_view pattern) {
    static thread_local std::vector<char> bytes;
    static thread_local std::vector<char> mask;

    bytes.clear();
    mask.clear();

    std::size_t i = 0;

    while (i < pattern.size()) {
        while (i < pattern.size() &&
               std::isspace(
                   static_cast<unsigned char>(pattern[i])
               )) {
            ++i;
        }

        if (i >= pattern.size()) {
            break;
        }

        if (pattern[i] == '?') {
            bytes.push_back(0);
            mask.push_back('?');

            ++i;

            if (i < pattern.size() &&
                pattern[i] == '?') {
                ++i;
            }

            continue;
        }

        if (i + 1 >= pattern.size()) {
            break;
        }

        const char h1 = pattern[i];
        const char h2 = pattern[i + 1];

        auto hex = [](char c) -> int {
            if (c >= '0' && c <= '9')
                return c - '0';

            if (c >= 'a' && c <= 'f')
                return c - 'a' + 10;

            if (c >= 'A' && c <= 'F')
                return c - 'A' + 10;

            return -1;
        };

        const int high = hex(h1);
        const int low  = hex(h2);

        if (high < 0 || low < 0) {
            ++i;
            continue;
        }

        bytes.push_back(
            static_cast<char>((high << 4) | low)
        );

        mask.push_back('x');

        i += 2;
    }

    return {
        bytes.data(),
        mask.data(),
        bytes.size()
    };
}

uintptr_t findPattern(
    uintptr_t start,
    std::size_t size,
    const Pattern& pattern
) {
    if (!start ||
        !size ||
        !pattern.bytes ||
        !pattern.mask ||
        !pattern.size) {
        return 0;
    }

    if (pattern.size > size) {
        return 0;
    }

    const auto* memory =
        reinterpret_cast<const uint8_t*>(start);

    for (std::size_t i = 0;
         i <= size - pattern.size;
         ++i) {

        bool matched = true;

        for (std::size_t j = 0;
             j < pattern.size;
             ++j) {

            if (pattern.mask[j] == '?') {
                continue;
            }

            if (memory[i + j] !=
                static_cast<uint8_t>(pattern.bytes[j])) {

                matched = false;
                break;
            }
        }

        if (matched) {
            return start + i;
        }
    }

    return 0;
}

uintptr_t findPattern(
    uintptr_t start,
    std::size_t size,
    std::string_view pattern
) {
    const Pattern parsed = parsePattern(pattern);

    return findPattern(
        start,
        size,
        parsed
    );
}

} // namespace levi::memory
