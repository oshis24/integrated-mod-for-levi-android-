#include "levi/minecraft/MatrixStack.hpp"

#include "levi/core/Logger.hpp"

namespace levi::minecraft {

void MatrixStack::bind(
    PushFn push,
    PopFn pop,
    TranslateFn translate,
    RotateFn rotate,
    ScaleFn scale
) noexcept {
    pushFn_ = push;
    popFn_ = pop;
    translateFn_ = translate;
    rotateFn_ = rotate;
    scaleFn_ = scale;

    levi::core::Logger::info(
        "MatrixStack native bridge bound"
    );
}

bool MatrixStack::bound() noexcept {
    return
        pushFn_ != nullptr &&
        popFn_ != nullptr &&
        translateFn_ != nullptr &&
        rotateFn_ != nullptr &&
        scaleFn_ != nullptr;
}

void MatrixStack::push() noexcept {
    if (!valid() || pushFn_ == nullptr) {
        return;
    }

    pushFn_(
        reinterpret_cast<void*>(address_)
    );
}

void MatrixStack::pop() noexcept {
    if (!valid() || popFn_ == nullptr) {
        return;
    }

    popFn_(
        reinterpret_cast<void*>(address_)
    );
}

void MatrixStack::translate(
    const levi::math::Vec3& value
) noexcept {
    if (!valid() || translateFn_ == nullptr) {
        return;
    }

    translateFn_(
        reinterpret_cast<void*>(address_),
        value.x,
        value.y,
        value.z
    );
}

void MatrixStack::rotate(
    const levi::math::Vec3& value
) noexcept {
    if (!valid() || rotateFn_ == nullptr) {
        return;
    }

    rotateFn_(
        reinterpret_cast<void*>(address_),
        value.x,
        value.y,
        value.z
    );
}

void MatrixStack::scale(
    const levi::math::Vec3& value
) noexcept {
    if (!valid() || scaleFn_ == nullptr) {
        return;
    }

    scaleFn_(
        reinterpret_cast<void*>(address_),
        value.x,
        value.y,
        value.z
    );
}

void MatrixStack::apply(
    const levi::math::Transform& transform
) noexcept {
    if (!valid()) {
        return;
    }

    /*
     * ViewModel transform order:
     *
     * pivot
     *   ↓
     * translation
     *   ↓
     * rotation
     *   ↓
     * scale
     *   ↓
     * inverse pivot
     *
     * This keeps the transformation local to the
     * current MatrixStack frame.
     */

    const bool hasPivot =
        transform.pivot.x != 0.0f ||
        transform.pivot.y != 0.0f ||
        transform.pivot.z != 0.0f;

    if (hasPivot) {
        translate(transform.pivot);
    }

    translate(transform.translation);

    rotate(transform.rotation);

    scale(transform.scale);

    if (hasPivot) {
        translate(
            transform.pivot * -1.0f
        );
    }
}

} // namespace levi::minecraft
