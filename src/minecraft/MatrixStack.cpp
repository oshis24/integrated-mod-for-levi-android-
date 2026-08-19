#include "levi/minecraft/MatrixStack.hpp"

#include <cmath>
#include <cstring>

namespace levi::minecraft {

namespace {

constexpr std::uintptr_t kRenderContextMatrixStackWrapper = 0x28;
constexpr std::uintptr_t kWrapperMatrixStack = 0x18;

constexpr std::uintptr_t kBlocksOffset = 0x50;
constexpr std::uintptr_t kStartOffset = 0x68;
constexpr std::uintptr_t kSizeOffset = 0x70;

constexpr std::size_t kMatrixBytes = 64;

constexpr float kPi = 3.14159265358979323846f;

void multiplyRight(
    Matrix4& matrix,
    const Matrix4& right
) noexcept {
    Matrix4 out{};

    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            float value = 0.0f;

            for (int k = 0; k < 4; ++k) {
                value +=
                    matrix.m[k * 4 + row] *
                    right.m[column * 4 + k];
            }

            out.m[column * 4 + row] = value;
        }
    }

    matrix = out;
}

Matrix4 identity() noexcept {
    Matrix4 result{};

    result.m[0] = 1.0f;
    result.m[5] = 1.0f;
    result.m[10] = 1.0f;
    result.m[15] = 1.0f;

    return result;
}

void translate(
    Matrix4& matrix,
    float x,
    float y,
    float z
) noexcept {
    Matrix4 transform = identity();

    transform.m[12] = x;
    transform.m[13] = y;
    transform.m[14] = z;

    multiplyRight(
        matrix,
        transform
    );
}

void scale(
    Matrix4& matrix,
    float x,
    float y,
    float z
) noexcept {
    Matrix4 transform = identity();

    transform.m[0] = x;
    transform.m[5] = y;
    transform.m[10] = z;

    multiplyRight(
        matrix,
        transform
    );
}

void rotateX(
    Matrix4& matrix,
    float degrees
) noexcept {
    const float radians =
        degrees * kPi / 180.0f;

    const float c = std::cos(radians);
    const float s = std::sin(radians);

    Matrix4 transform = identity();

    transform.m[5] = c;
    transform.m[6] = s;
    transform.m[9] = -s;
    transform.m[10] = c;

    multiplyRight(
        matrix,
        transform
    );
}

void rotateY(
    Matrix4& matrix,
    float degrees
) noexcept {
    const float radians =
        degrees * kPi / 180.0f;

    const float c = std::cos(radians);
    const float s = std::sin(radians);

    Matrix4 transform = identity();

    transform.m[0] = c;
    transform.m[2] = -s;
    transform.m[8] = s;
    transform.m[10] = c;

    multiplyRight(
        matrix,
        transform
    );
}

void rotateZ(
    Matrix4& matrix,
    float degrees
) noexcept {
    const float radians =
        degrees * kPi / 180.0f;

    const float c = std::cos(radians);
    const float s = std::sin(radians);

    Matrix4 transform = identity();

    transform.m[0] = c;
    transform.m[1] = s;
    transform.m[4] = -s;
    transform.m[5] = c;

    multiplyRight(
        matrix,
        transform
    );
}

} // namespace

MatrixStack MatrixStack::fromRenderContext(
    void* renderContext
) noexcept {
    if (renderContext == nullptr) {
        return {};
    }

    const auto context =
        reinterpret_cast<std::uintptr_t>(
            renderContext
        );

    const auto wrapper =
        *reinterpret_cast<
            const std::uintptr_t*
        >(
            context +
            kRenderContextMatrixStackWrapper
        );

    if (wrapper == 0) {
        return {};
    }

    const auto stack =
        *reinterpret_cast<
            const std::uintptr_t*
        >(
            wrapper +
            kWrapperMatrixStack
        );

    return MatrixStack(stack);
}

Matrix4* MatrixStack::current() const noexcept {
    if (!valid()) {
        return nullptr;
    }

    auto* blocks =
        *reinterpret_cast<
            std::uintptr_t**
        >(
            address_ +
            kBlocksOffset
        );

    const auto start =
        *reinterpret_cast<
            const std::size_t*
        >(
            address_ +
            kStartOffset
        );

    const auto size =
        *reinterpret_cast<
            const std::size_t*
        >(
            address_ +
            kSizeOffset
        );

    if (
        blocks == nullptr ||
        size == 0
    ) {
        return nullptr;
    }

    /*
     * Same MatrixStack indexing used by the working
     * BedrockTools ViewModel.
     */
    const std::size_t last =
        start + size - 1;

    const std::size_t blockOffset =
        (last >> 3) &
        ~static_cast<std::size_t>(7);

    const std::size_t elementIndex =
        last & 0x3F;

    const auto block =
        *reinterpret_cast<
            const std::uintptr_t*
        >(
            reinterpret_cast<
                std::uintptr_t
            >(blocks) +
            blockOffset
        );

    if (block == 0) {
        return nullptr;
    }

    return reinterpret_cast<Matrix4*>(
        block +
        elementIndex *
            kMatrixBytes
    );
}

bool MatrixStack::snapshot(
    Matrix4& out
) const noexcept {
    const auto* matrix = current();

    if (matrix == nullptr) {
        return false;
    }

    std::memcpy(
        &out,
        matrix,
        sizeof(Matrix4)
    );

    return true;
}

bool MatrixStack::restore(
    const Matrix4& value
) const noexcept {
    auto* matrix = current();

    if (matrix == nullptr) {
        return false;
    }

    std::memcpy(
        matrix,
        &value,
        sizeof(Matrix4)
    );

    return true;
}

bool MatrixStack::apply(
    const levi::math::Transform& transform
) const noexcept {
    auto* matrix = current();

    if (matrix == nullptr) {
        return false;
    }

    if (
        transform.translation.x != 0.0f ||
        transform.translation.y != 0.0f ||
        transform.translation.z != 0.0f
    ) {
        translate(
            *matrix,
            transform.translation.x,
            transform.translation.y,
            transform.translation.z
        );
    }

    const bool hasRotation =
        transform.rotation.x != 0.0f ||
        transform.rotation.y != 0.0f ||
        transform.rotation.z != 0.0f;

    if (hasRotation) {
        const bool hasPivot =
            transform.pivot.x != 0.0f ||
            transform.pivot.y != 0.0f ||
            transform.pivot.z != 0.0f;

        if (hasPivot) {
            translate(
                *matrix,
                transform.pivot.x,
                transform.pivot.y,
                transform.pivot.z
            );
        }

        if (transform.rotation.x != 0.0f) {
            rotateX(
                *matrix,
                transform.rotation.x
            );
        }

        if (transform.rotation.y != 0.0f) {
            rotateY(
                *matrix,
                transform.rotation.y
            );
        }

        if (transform.rotation.z != 0.0f) {
            rotateZ(
                *matrix,
                transform.rotation.z
            );
        }

        if (hasPivot) {
            translate(
                *matrix,
                -transform.pivot.x,
                -transform.pivot.y,
                -transform.pivot.z
            );
        }
    }

    if (
        transform.scale.x != 1.0f ||
        transform.scale.y != 1.0f ||
        transform.scale.z != 1.0f
    ) {
        scale(
            *matrix,
            transform.scale.x,
            transform.scale.y,
            transform.scale.z
        );
    }

    return true;
}

} // namespace levi::minecraft
