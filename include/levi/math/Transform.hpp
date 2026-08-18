#pragma once

#include "levi/math/Vec3.hpp"

namespace levi::math {

struct Transform {
    Vec3 translation{
        0.0f,
        0.0f,
        0.0f
    };

    Vec3 rotation{
        0.0f,
        0.0f,
        0.0f
    };

    Vec3 scale{
        1.0f,
        1.0f,
        1.0f
    };

    Vec3 pivot{
        0.0f,
        0.0f,
        0.0f
    };

    static Transform identity() noexcept {
        return {};
    }

    void reset() noexcept {
        translation = {};
        rotation = {};

        scale = {
            1.0f,
            1.0f,
            1.0f
        };

        pivot = {};
    }

    bool isIdentity() const noexcept {
        return
            translation.x == 0.0f &&
            translation.y == 0.0f &&
            translation.z == 0.0f &&

            rotation.x == 0.0f &&
            rotation.y == 0.0f &&
            rotation.z == 0.0f &&

            scale.x == 1.0f &&
            scale.y == 1.0f &&
            scale.z == 1.0f &&

            pivot.x == 0.0f &&
            pivot.y == 0.0f &&
            pivot.z == 0.0f;
    }
};

} // namespace levi::math
