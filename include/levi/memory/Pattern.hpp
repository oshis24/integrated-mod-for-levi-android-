#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace levi::memory {

class Pattern final {
public:
    Pattern() = default;

    explicit Pattern(
        std::string_view pattern
    );

    Pattern(
        const std::uint8_t* bytes,
        const std::uint8_t* masks,
        std::size_t size
    );

    bool valid() const noexcept;

    std::size_t size() const noexcept;

    bool matches(
        const std::uint8_t* address
    ) const noexcept;

private:
    std::vector<std::uint8_t> bytes_;
    std::vector<std::uint8_t> masks_;

public:
    const std::vector<std::uint8_t>&
    bytes() const noexcept;

    const std::vector<std::uint8_t>&
    masks() const noexcept;
};

} // namespace levi::memory
