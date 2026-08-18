#pragma once

#include "levi/modules/Module.hpp"

namespace levi::modules {

class ViewModel final : public Module {
public:
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

private:
    bool enabled_{false};

    ModuleStatus status_{
        ModuleStatus::Disabled
    };
};

} // namespace levi::modules
