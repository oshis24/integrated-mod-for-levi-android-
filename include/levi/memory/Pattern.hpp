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

    bool valid() const noexcept;

    std::size_t size() const noexcept;

    bool matches(
        const std::uint8_t* address
    ) const noexcept;

private:
    std::vector<std::uint8_t> bytes_;
    std::vector<bool> mask_;

public:
    const std::vector<std::uint8_t>& bytes()
        const noexcept;

    const std::vector<bool>& mask()
        const noexcept;
};

} // namespace levi::memory
