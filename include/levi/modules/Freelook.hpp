#pragma once

#include "levi/modules/Module.hpp"

namespace levi::modules {

class Freelook final : public Module {
public:
    ModuleId id() const noexcept override {
        return ModuleId::Freelook;
    }

    const char* name() const noexcept override {
        return "Freelook";
    }

    bool initialize() noexcept override;

    void shutdown() noexcept override;

    void tick(float deltaTime) noexcept override;

    bool enable() noexcept override;

    void disable() noexcept override;

    bool enabled() const noexcept override;

    ModuleStatus status() const noexcept override;

    float yaw() const noexcept {
        return yawOffset_;
    }

    float pitch() const noexcept {
        return pitchOffset_;
    }

    void resetRotation() noexcept;

    void setSensitivity(
        float value
    ) noexcept {
        sensitivity_ = value;
    }

    float sensitivity() const noexcept {
        return sensitivity_;
    }

private:
    static bool turnDeltaHandler(
        float x,
        float y
    ) noexcept;

    static int perspectiveOverride(
        int originalPerspective
    ) noexcept;

    static bool visualRotationProvider(
        float vanillaPitch,
        float vanillaYaw,
        float& outPitch,
        float& outYaw
    ) noexcept;

    static float wrapYaw(
        float value
    ) noexcept;

private:
    inline static Freelook*
        active_{nullptr};

    bool enabled_{false};

    bool visualInitialized_{false};

    float basePitch_{0.0f};
    float baseYaw_{0.0f};

    float pitchOffset_{0.0f};
    float yawOffset_{0.0f};

    float sensitivity_{1.0f};

    int lockedPerspective_{0};

    ModuleStatus status_{
        ModuleStatus::Disabled
    };
};

} // namespace levi::modules
