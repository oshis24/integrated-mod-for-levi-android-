#pragma once

#include "levi/math/Transform.hpp"

#include <cstdint>

namespace levi::minecraft {

class MatrixStack final {
public:
    using PushFn = void(*)(
        void* self
    );

    using PopFn = void(*)(
        void* self
    );

    using TranslateFn = void(*)(
        void* self,
        float x,
        float y,
        float z
    );

    using RotateFn = void(*)(
        void* self,
        float x,
        float y,
        float z
    );

    using ScaleFn = void(*)(
        void* self,
        float x,
        float y,
        float z
    );

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
     * Native bindings.
     *
     * These are populated by the Minecraft 1.26.44.3
     * rendering bridge once the corresponding targets
     * have been resolved.
     */
    static void bind(
        PushFn push,
        PopFn pop,
        TranslateFn translate,
        RotateFn rotate,
        ScaleFn scale
    ) noexcept;

    static bool bound() noexcept;

    void push() noexcept;

    void pop() noexcept;

    void translate(
        const levi::math::Vec3& value
    ) noexcept;

    void rotate(
        const levi::math::Vec3& value
    ) noexcept;

    void scale(
        const levi::math::Vec3& value
    ) noexcept;

    void apply(
        const levi::math::Transform& transform
    ) noexcept;

private:
    std::uintptr_t address_{0};

    inline static PushFn pushFn_{nullptr};
    inline static PopFn popFn_{nullptr};
    inline static TranslateFn translateFn_{nullptr};
    inline static RotateFn rotateFn_{nullptr};
    inline static ScaleFn scaleFn_{nullptr};
};

} // namespace levi::minecraft
