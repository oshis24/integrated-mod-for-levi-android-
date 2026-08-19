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
        return yaw_;
    }

    float pitch() const noexcept {
        return pitch_;
    }

    void resetRotation() noexcept;

    void setSensitivity(
        float value
    ) noexcept {
        sensitivity_ = value;
    }

private:
    static bool turnDeltaHandler(
        float x,
        float y
    ) noexcept;

    static int perspectiveOverride(
        int originalPerspective
    ) noexcept;

private:
    inline static Freelook*
        active_{nullptr};

    bool enabled_{false};

    float yaw_{0.0f};
    float pitch_{0.0f};
    float sensitivity_{1.0f};

    int lockedPerspective_{0};

    ModuleStatus status_{
        ModuleStatus::Disabled
    };
};

} // namespace levi::modules
