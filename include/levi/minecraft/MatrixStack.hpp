#pragma once

#include "levi/math/Transform.hpp"

#include <cstdint>

namespace levi::minecraft {

struct Matrix4 final {
    float m[16]{};
};

class MatrixStack final {
public:
    MatrixStack() = default;

    explicit MatrixStack(
        std::uintptr_t address
    ) noexcept
        : address_(address) {
    }

    bool valid() const noexcept {
        return address_ != 0;
    }

    std::uintptr_t address() const noexcept {
        return address_;
    }

    static MatrixStack fromRenderContext(
        void* renderContext
    ) noexcept;

    Matrix4* current() const noexcept;

    bool snapshot(
        Matrix4& out
    ) const noexcept;

    bool restore(
        const Matrix4& value
    ) const noexcept;

    bool apply(
        const levi::math::Transform& transform
    ) const noexcept;

private:
    std::uintptr_t address_{0};
};

} // namespace levi::minecraft
