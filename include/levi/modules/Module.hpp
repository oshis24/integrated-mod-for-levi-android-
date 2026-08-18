#pragma once

#include <cstdint>

namespace levi::modules {

enum class ModuleId : std::uint8_t {
    ViewModel = 0,
    Freelook = 1,
    ItemPhysics = 2
};

enum class ModuleStatus : std::uint8_t {
    Disabled = 0,
    WaitingForRuntime,
    WaitingForTarget,
    Ready,
    Active,
    Failed
};

class Module {
public:
    virtual ~Module() = default;

    virtual ModuleId id() const noexcept = 0;

    virtual const char* name() const noexcept = 0;

    virtual bool initialize() noexcept = 0;

    virtual void shutdown() noexcept = 0;

    virtual void tick(float deltaTime) noexcept = 0;

    virtual bool enable() noexcept = 0;

    virtual void disable() noexcept = 0;

    virtual bool enabled() const noexcept = 0;

    virtual ModuleStatus status() const noexcept = 0;
};

} // namespace levi::modules
