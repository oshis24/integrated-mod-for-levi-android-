#pragma once

#include "levi/math/Vec3.hpp"

#include <cstdint>

namespace levi::minecraft {

/*
 * Opaque MatrixStack abstraction.
 *
 * ViewModel will eventually manipulate this object through the
 * native Bedrock methods discovered during RE.
 *
 * DO NOT assume the fields below represent the actual Bedrock
 * memory layout.
 */
class MatrixStack final {
public:
    MatrixStack() = default;

    explicit MatrixStack(
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

    /*
     * These functions are intentionally declarations of the
     * abstraction rather than direct memory manipulation.
     *
     * Their native targets will be attached after the
     * 1.26.44.3 MatrixStack RE is finalized.
     */

    void translate(
        const levi::math::Vec3& value
    ) noexcept;

    void rotate(
        const levi::math::Vec3& value
    ) noexcept;

    void scale(
        const levi::math::Vec3& value
    ) noexcept;

private:
    std::uintptr_t address_{0};
};

} // namespace levi::minecraft
