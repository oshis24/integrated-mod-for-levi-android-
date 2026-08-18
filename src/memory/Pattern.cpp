#include "levi/memory/Pattern.hpp"

#include <cstdlib>
#include <string>

namespace levi::memory {

namespace {

bool parseByte(
    std::string_view token,
    std::uint8_t& value,
    std::uint8_t& mask
) noexcept {
    if (
        token == "?" ||
        token == "??"
    ) {
        value = 0;
        mask = 0;
        return true;
    }

    if (token.size() != 2) {
        return false;
    }

    char* end = nullptr;

    const unsigned long parsed =
        std::strtoul(
            std::string(token).c_str(),
            &end,
            16
        );

    if (
        end == nullptr ||
        *end != '\0' ||
        parsed > 0xff
    ) {
        return false;
    }

    value =
        static_cast<std::uint8_t>(
            parsed
        );

    mask = 0xff;

    return true;
}

} // namespace

Pattern::Pattern(
    std::string_view pattern
) {
    std::size_t position = 0;

    while (position < pattern.size()) {
        while (
            position < pattern.size() &&
            pattern[position] == ' '
        ) {
            ++position;
        }

        if (position >= pattern.size()) {
            break;
        }

        const std::size_t begin =
            position;

        while (
            position < pattern.size() &&
            pattern[position] != ' '
        ) {
            ++position;
        }

        const auto token =
            pattern.substr(
                begin,
                position - begin
            );

        std::uint8_t value = 0;
        std::uint8_t mask = 0;

        if (
            !parseByte(
                token,
                value,
                mask
            )
        ) {
            bytes_.clear();
            masks_.clear();
            return;
        }

        bytes_.push_back(value);
        masks_.push_back(mask);
    }
}

Pattern::Pattern(
    const std::uint8_t* bytes,
    const std::uint8_t* masks,
    std::size_t size
) {
    if (
        bytes == nullptr ||
        masks == nullptr ||
        size == 0
    ) {
        return;
    }

    bytes_.assign(
        bytes,
        bytes + size
    );

    masks_.assign(
        masks,
        masks + size
    );
}

bool Pattern::valid() const noexcept {
    return
        !bytes_.empty() &&
        bytes_.size() == masks_.size();
}

std::size_t Pattern::size() const noexcept {
    return bytes_.size();
}

bool Pattern::matches(
    const std::uint8_t* address
) const noexcept {
    if (
        address == nullptr ||
        !valid()
    ) {
        return false;
    }

    for (
        std::size_t i = 0;
        i < bytes_.size();
        ++i
    ) {
        /*
         * Reference ItemPhysics uses exact-byte signatures.
         *
         * mask == 0:
         *     wildcard
         *
         * mask == 0xff:
         *     exact byte
         */
        if (
            (address[i] & masks_[i]) !=
            (bytes_[i] & masks_[i])
        ) {
            return false;
        }
    }

    return true;
}

const std::vector<std::uint8_t>&
Pattern::bytes() const noexcept {
    return bytes_;
}

const std::vector<std::uint8_t>&
Pattern::masks() const noexcept {
    return masks_;
}

} // namespace levi::memory
