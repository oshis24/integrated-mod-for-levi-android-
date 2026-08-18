#pragma once

#include "levi/math/Transform.hpp"
#include "levi/modules/Module.hpp"

#include <cstdint>

namespace levi::modules {

enum class ItemVisualType : std::uint8_t {
    Unknown = 0,
    FlatItem,
    BlockItem,
    Tool,
    Weapon,
    Shield,
    Banner
};

struct ItemPhysicsTransform {
    levi::math::Transform transform{};

    bool replaceVanillaSpin{true};

    ItemVisualType visualType{
        ItemVisualType::Unknown
    };
};

class ItemPhysics final : public Module {
public:
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

    ItemPhysicsTransform transformFor(
        ItemVisualType type
    ) const noexcept;

private:
    bool enabled_{false};

    ModuleStatus status_{
        ModuleStatus::Disabled
    };
};

} // namespace levi::modules
