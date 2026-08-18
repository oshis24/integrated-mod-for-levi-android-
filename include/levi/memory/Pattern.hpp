#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace levi::memory {

class Pattern final {
public:
    Pattern() = default;

    /*
     * Pattern format:
     *
     *     "AA BB CC ?? DD EE"
     *
     * ?? means wildcard.
     */
    explicit Pattern(const char* pattern);

    explicit Pattern(const std::string& pattern);

    bool valid() const noexcept;

    const std::vector<std::uint8_t>& bytes() const noexcept;
    const std::vector<bool>& mask() const noexcept;

    std::size_t size() const noexcept;

    bool matches(
        const std::uint8_t* address
    ) const noexcept;

private:
    std::vector<std::uint8_t> bytes_;
    std::vector<bool> mask_;
};

} // namespace levi::memory
