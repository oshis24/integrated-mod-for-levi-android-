#pragma once

#include "levi/minecraft/MatrixStack.hpp"
#include "levi/minecraft/RenderContext.hpp"

#include <cstddef>
#include <cstdint>

namespace levi::minecraft {

class ItemRenderer final {
public:
    /*
     * Hasil RE kita saat ini:
     *
     * ItemInHandRenderer
     *        |
     *        +-- vtable + 0x18
     *
     * 0x18 / 8 = 3
     *
     * Jadi entry yang kita intercept adalah index 3.
     *
     * ABI callback TIDAK boleh dianggap final sebelum binary
     * target memberikan boundary lengkap. Karena itu callback
     * menggunakan ABI yang sudah digunakan repo saat ini.
     */
    static constexpr std::size_t
        kRenderFirstPersonIndex = 3;

    using RenderFirstPersonFn =
        void(*)(
            void* self,
            RenderContext* context,
            MatrixStack* matrixStack
        );

public:
    static bool attach(
        void* renderer
    ) noexcept;

    static bool detach() noexcept;

    static bool attached() noexcept;

    static void* renderer() noexcept;

    static void setViewModelEnabled(
        bool enabled
    ) noexcept;

    static bool viewModelEnabled() noexcept;

    static void setViewModelTransform(
        const levi::math::Transform& transform
    ) noexcept;

    static const levi::math::Transform&
    viewModelTransform() noexcept;

private:
    static void hookRenderFirstPerson(
        void* self,
        RenderContext* context,
        MatrixStack* matrixStack
    ) noexcept;

private:
    inline static void* renderer_{nullptr};

    inline static void*
        originalRenderFirstPerson_{nullptr};

    inline static bool attached_{false};

    inline static bool
        viewModelEnabled_{false};

    inline static levi::math::Transform
        viewModelTransform_{};
};

} // namespace levi::minecraft
