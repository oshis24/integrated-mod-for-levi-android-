#include "levi/memory/Pattern.hpp"

#include <cstdlib>
#include <string>

namespace levi::memory {

namespace {

bool parseByte(
    std::string_view token,
    std::uint8_t& value,
    bool& wildcard
) noexcept {
    if (token == "?" || token == "??") {
        value = 0;
        wildcard = true;
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
        parsed > 0xFF
    ) {
        return false;
    }

    value =
        static_cast<std::uint8_t>(parsed);

    wildcard = false;
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

        const std::size_t begin = position;

        while (
            position < pattern.size() &&
            pattern[position] != ' '
        ) {
            ++position;
        }

        const std::string_view token =
            pattern.substr(
                begin,
                position - begin
            );

        std::uint8_t value = 0;
        bool wildcard = false;

        if (
            !parseByte(
                token,
                value,
                wildcard
            )
        ) {
            bytes_.clear();
            mask_.clear();
            return;
        }

        bytes_.push_back(value);
        mask_.push_back(!wildcard);
    }
}

bool Pattern::valid() const noexcept {
    return
        !bytes_.empty() &&
        bytes_.size() == mask_.size();
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
        if (!mask_[i]) {
            continue;
        }

        if (address[i] != bytes_[i]) {
            return false;
        }
    }

    return true;
}

const std::vector<std::uint8_t>&
Pattern::bytes() const noexcept {
    return bytes_;
}

const std::vector<bool>&
Pattern::mask() const noexcept {
    return mask_;
}

} // namespace levi::memory
