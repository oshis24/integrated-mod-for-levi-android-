#pragma once

#include "levi/math/Vec3.hpp"

#include <cstdint>

namespace levi::minecraft {

class ItemEntity final {
public:
    ItemEntity() = default;

    explicit ItemEntity(
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
     * Runtime state.
     *
     * These are deliberately cached values rather than assumed
     * native memory offsets.
     */
    const levi::math::Vec3& position() const noexcept {
        return position_;
    }

    const levi::math::Vec3& velocity() const noexcept {
        return velocity_;
    }

    void setPosition(
        const levi::math::Vec3& value
    ) noexcept {
        position_ = value;
    }

    void setVelocity(
        const levi::math::Vec3& value
    ) noexcept {
        velocity_ = value;
    }

    float age() const noexcept {
        return age_;
    }

    void setAge(float value) noexcept {
        age_ = value;
    }

private:
    std::uintptr_t address_{0};

    levi::math::Vec3 position_{};
    levi::math::Vec3 velocity_{};

    float age_{0.0f};
};

} // namespace levi::minecraft
