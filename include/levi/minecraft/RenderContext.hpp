#pragma once

#include <cstdint>

namespace levi::minecraft {

/*
 * Opaque wrapper around Bedrock's native RenderContext.
 *
 * We deliberately do not define the internal layout yet.
 *
 * Reason:
 * RenderContext is a native engine object and its layout must
 * be reconstructed from the exact Minecraft 1.26.44.3 binary.
 */
class RenderContext final {
public:
    RenderContext() = default;

    explicit RenderContext(
        std::uintptr_t address
    )
        : address_(address) {
    }

    bool valid() const noexcept {
        return address_ != 0;
    }

    std::uintptr_t address() const noexcept {
        return address_;
    }

    void reset() noexcept {
        address_ = 0;
    }

private:
    std::uintptr_t address_{0};
};

} // namespace levi::minecraft
