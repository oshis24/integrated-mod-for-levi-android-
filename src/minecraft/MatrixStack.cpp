#include "levi/minecraft/MatrixStack.hpp"

#include "levi/core/Logger.hpp"

namespace levi::minecraft {

void MatrixStack::translate(
    const levi::math::Vec3& value
) noexcept {
    if (!valid()) {
        return;
    }

    /*
     * Native target belum di-bind.
     *
     * Nanti:
     *
     * MatrixStack::translate(...)
     *        ↓
     * Bedrock MatrixStack native method
     *
     * Target akan diambil dari profile 1.26.44.3.
     */

    (void)value;

    levi::core::Logger::debug(
        "MatrixStack::translate() called before native binding"
    );
}

void MatrixStack::rotate(
    const levi::math::Vec3& value
) noexcept {
    if (!valid()) {
        return;
    }

    (void)value;

    levi::core::Logger::debug(
        "MatrixStack::rotate() called before native binding"
    );
}

void MatrixStack::scale(
    const levi::math::Vec3& value
) noexcept {
    if (!valid()) {
        return;
    }

    (void)value;

    levi::core::Logger::debug(
        "MatrixStack::scale() called before native binding"
    );
}

} // namespace levi::minecraft
