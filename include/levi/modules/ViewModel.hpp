#pragma once

#include "levi/math/Transform.hpp"
#include "levi/memory/Hook.hpp"
#include "levi/modules/Module.hpp"

#include <cstdint>

namespace levi::modules {

class ViewModel final : public Module {
public:
    using GetFovFn =
        float(*)(
            void* self,
            float value,
            int enableVariableFov
        );

    ModuleId id() const noexcept override {
        return ModuleId::ViewModel;
    }

    const char* name() const noexcept override {
        return "ViewModel";
    }

    bool initialize() noexcept override;
    void shutdown() noexcept override;
    void tick(float deltaTime) noexcept override;

    bool enable() noexcept override;
    void disable() noexcept override;

    bool enabled() const noexcept override;
    ModuleStatus status() const noexcept override;

    void bindNativeTarget(
        std::uintptr_t getFov
    ) noexcept;

    const levi::math::Transform&
    transform() const noexcept {
        return transform_;
    }

    void setTransform(
        const levi::math::Transform& value
    ) noexcept {
        transform_ = value;
    }

    void resetTransform() noexcept {
        transform_.reset();
    }

    float itemFov() const noexcept {
        return itemFov_;
    }

    void setItemFov(
        float value
    ) noexcept {
        itemFov_ = value;
    }

    bool applyThirdPerson() const noexcept {
        return applyThirdPerson_;
    }

    void setApplyThirdPerson(
        bool value
    ) noexcept {
        applyThirdPerson_ = value;
    }

private:
    static float fovDetour(
        void* self,
        float value,
        int enableVariableFov
    ) noexcept;

    static void onPerspective(
        int perspective
    ) noexcept;

private:
    inline static ViewModel*
        active_{nullptr};

    levi::memory::Hook fovHook_{};

    bool enabled_{false};
    bool thirdPerson_{false};
    bool applyThirdPerson_{false};

    float itemFov_{70.0f};

    ModuleStatus status_{
        ModuleStatus::Disabled
    };

    levi::math::Transform transform_{};
};

} // namespace levi::modules
