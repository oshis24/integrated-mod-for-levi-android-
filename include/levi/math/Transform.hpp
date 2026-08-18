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
};

} // namespace levi::math
