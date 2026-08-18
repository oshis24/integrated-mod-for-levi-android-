#pragma once

#include "levi/math/Vec3.hpp"

#include <cstdint>

namespace levi::minecraft {

class Camera final {
public:
    Camera() = default;

    explicit Camera(
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
     * Camera state used by Freelook.
     *
     * These are client-side values, not assumed native
     * object offsets.
     */
    float yaw() const noexcept {
        return yaw_;
    }

    float pitch() const noexcept {
        return pitch_;
    }

    void setYaw(float value) noexcept {
        yaw_ = value;
    }

    void setPitch(float value) noexcept {
        pitch_ = value;
    }

    void addRotation(
        float yawDelta,
        float pitchDelta
    ) noexcept;

    void resetRotation() noexcept;

private:
    std::uintptr_t address_{0};

    float yaw_{0.0f};
    float pitch_{0.0f};
};

} // namespace levi::minecraft
