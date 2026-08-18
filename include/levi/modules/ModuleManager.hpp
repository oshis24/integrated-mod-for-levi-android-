#pragma once

#include "levi/modules/Module.hpp"

#include <array>
#include <cstddef>

namespace levi::modules {

class ModuleManager final {
public:
    static ModuleManager& instance();

    bool initialize() noexcept;

    void shutdown() noexcept;

    void tick(float deltaTime) noexcept;

    Module* get(ModuleId id) noexcept;

    const Module* get(ModuleId id) const noexcept;

    bool enable(ModuleId id) noexcept;

    void disable(ModuleId id) noexcept;

    bool initialized() const noexcept;

private:
    ModuleManager();

    ~ModuleManager() = default;

    ModuleManager(const ModuleManager&) = delete;
    ModuleManager& operator=(const ModuleManager&) = delete;

private:
    std::array<Module*, 3> modules_{};

    bool initialized_{false};
};

} // namespace levi::modules
