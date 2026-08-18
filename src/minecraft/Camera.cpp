#include "levi/minecraft/Camera.hpp"

#include <algorithm>

namespace levi::minecraft {

void Camera::addRotation(
    float yawDelta,
    float pitchDelta
) noexcept {
    yaw_ += yawDelta;
    pitch_ += pitchDelta;

    /*
     * Bedrock pitch is normally constrained vertically.
     *
     * We keep the abstraction bounded here so Freelook cannot
     * accidentally generate invalid camera values.
     */
    pitch_ = std::clamp(
        pitch_,
        -90.0f,
        90.0f
    );
}

void Camera::resetRotation() noexcept {
    yaw_ = 0.0f;
    pitch_ = 0.0f;
}

} // namespace levi::minecraft
