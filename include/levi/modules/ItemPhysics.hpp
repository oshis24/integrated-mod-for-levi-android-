#pragma once

#include "levi/memory/Hook.hpp"
#include "levi/minecraft/MatrixStack.hpp"
#include "levi/modules/Module.hpp"

#include <array>
#include <cstdint>

namespace levi::modules {

class ItemPhysics final : public Module {
public:
    using SetupAndRenderFn =
        void(*)(
            void* self,
            void* context
        );

    /*
     * ABI reconstructed from the working ItemPhysic
     * detour.
     */
    using RenderItemGroupFn =
        void(*)(
            void* self,
            void* context,
            void* worldItem,
            int state,
            bool flag,
            float value0,
            float value1
        );

    ModuleId id() const noexcept override {
        return ModuleId::ItemPhysics;
    }

    const char* name() const noexcept override {
        return "ItemPhysics";
    }

    bool initialize() noexcept override;
    void shutdown() noexcept override;
    void tick(float deltaTime) noexcept override;

    bool enable() noexcept override;
    void disable() noexcept override;

    bool enabled() const noexcept override;
    ModuleStatus status() const noexcept override;

    void bindNativeTargets(
        std::uintptr_t setupAndRender,
        std::uintptr_t renderItemGroup
    ) noexcept;

    bool hooksInstalled() const noexcept;

private:
    struct OrientationEntry final {
        void* key{nullptr};

        float basis[9]{};

        std::uint64_t age{0};

        bool valid{false};
    };

    static void setupAndRenderDetour(
        void* self,
        void* context
    ) noexcept;

    static void renderItemGroupDetour(
        void* self,
        void* context,
        void* worldItem,
        int state,
        bool flag,
        float value0,
        float value1
    ) noexcept;

    static void worldTransformCallback(
        void* worldItem,
        levi::minecraft::MatrixStack& matrixStack
    ) noexcept;

    void applyStableOrientation(
        void* worldItem,
        levi::minecraft::MatrixStack& matrixStack
    ) noexcept;

    OrientationEntry* findOrCreateEntry(
        void* key
    ) noexcept;

    void expireOldEntries() noexcept;

private:
    inline static ItemPhysics*
        active_{nullptr};

    levi::memory::Hook
        setupHook_{};

    levi::memory::Hook
        groupHook_{};

    std::array<
        OrientationEntry,
        128
    > orientationCache_{};

    std::uint64_t frameCounter_{0};

    bool enabled_{false};

    ModuleStatus status_{
        ModuleStatus::Disabled
    };
};

} // namespace levi::modules
