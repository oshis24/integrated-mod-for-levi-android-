#pragma once

#include <cstddef>
#include <cstdint>

namespace levi::memory {

class VTable final {
public:
    static void* get(
        void* object,
        std::size_t index
    ) noexcept;

    static bool replace(
        void* object,
        std::size_t index,
        void* replacement,
        void** original
    ) noexcept;
};

} // namespace levi::memory
