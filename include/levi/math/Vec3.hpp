#pragma once

#include <cmath>

namespace levi::math {

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};

    constexpr Vec3() = default;

    constexpr Vec3(
        float xValue,
        float yValue,
        float zValue
    )
        : x(xValue),
          y(yValue),
          z(zValue) {
    }

    constexpr Vec3 operator+(
        const Vec3& other
    ) const noexcept {
        return {
            x + other.x,
            y + other.y,
            z + other.z
        };
    }

    constexpr Vec3 operator-(
        const Vec3& other
    ) const noexcept {
        return {
            x - other.x,
            y - other.y,
            z - other.z
        };
    }

    constexpr Vec3 operator*(
        float value
    ) const noexcept {
        return {
            x * value,
            y * value,
            z * value
        };
    }

    constexpr Vec3& operator+=(
        const Vec3& other
    ) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr Vec3& operator-=(
        const Vec3& other
    ) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr Vec3& operator*=(
        float value
    ) noexcept {
        x *= value;
        y *= value;
        z *= value;
        return *this;
    }

    float lengthSquared() const noexcept {
        return
            x * x +
            y * y +
            z * z;
    }

    float length() const noexcept {
        return std::sqrt(lengthSquared());
    }

    Vec3 normalized() const noexcept {
        const float len = length();

        if (len <= 0.000001f) {
            return {};
        }

        const float inverse = 1.0f / len;

        return {
            x * inverse,
            y * inverse,
            z * inverse
        };
    }
};

} // namespace levi::math
