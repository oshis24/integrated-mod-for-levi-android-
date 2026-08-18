#include "levi/memory/Pattern.hpp"

#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

namespace levi::memory {

namespace {

bool parseByte(
    const std::string& token,
    std::uint8_t& value,
    bool& wildcard
) {
    if (token == "?" || token == "??") {
        value = 0;
        wildcard = true;
        return true;
    }

    if (token.size() != 2) {
        return false;
    }

    const char* begin = token.data();
    char* end = nullptr;

    const unsigned long parsed =
        std::strtoul(begin, &end, 16);

    if (end != begin + 2 || parsed > 0xFF) {
        return false;
    }

    value = static_cast<std::uint8_t>(parsed);
    wildcard = false;

    return true;
}

} // namespace

Pattern::Pattern(const char* pattern) {
    if (pattern == nullptr) {
        return;
    }

    std::istringstream stream(pattern);
    std::string token;

    while (stream >> token) {
        std::uint8_t value = 0;
        bool wildcard = false;

        if (!parseByte(token, value, wildcard)) {
            bytes_.clear();
            mask_.clear();
            return;
        }

        bytes_.push_back(value);
        mask_.push_back(!wildcard);
    }
}

Pattern::Pattern(const std::string& pattern)
    : Pattern(pattern.c_str()) {
}

bool Pattern::valid() const noexcept {
    return !bytes_.empty() &&
           bytes_.size() == mask_.size();
}

const std::vector<std::uint8_t>& Pattern::bytes() const noexcept {
    return bytes_;
}

const std::vector<bool>& Pattern::mask() const noexcept {
    return mask_;
}

std::size_t Pattern::size() const noexcept {
    return bytes_.size();
}

bool Pattern::matches(
    const std::uint8_t* address
) const noexcept {

    if (!valid() || address == nullptr) {
        return false;
    }

    for (std::size_t i = 0; i < bytes_.size(); ++i) {
        if (!mask_[i]) {
            continue;
        }

        if (address[i] != bytes_[i]) {
            return false;
        }
    }

    return true;
}

} // namespace levi::memory
