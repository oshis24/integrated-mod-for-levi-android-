#include "levi/modules/ViewModel.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"
#include "levi/core/State.hpp"
#include "levi/minecraft/MinecraftProfile.hpp"

namespace levi::modules {

bool ViewModel::initialize() noexcept {
    if (
        status_ != ModuleStatus::Disabled &&
        status_ != ModuleStatus::Failed
    ) {
        return true;
    }

    auto& runtime =
        levi::core::Runtime::instance();

    if (!runtime.isInitialized()) {
        status_ = ModuleStatus::WaitingForRuntime;

        levi::core::Logger::debug(
            "ViewModel: runtime is not initialized"
        );

        return false;
    }

    if (!runtime.isSupportedMinecraftVersion()) {
        status_ = ModuleStatus::Failed;

        levi::core::Logger::error(
            "ViewModel: unsupported Minecraft version"
        );

        return false;
    }

    transform_.reset();

    /*
     * Default ViewModel transform.
     *
     * Kept neutral until the native render callback is attached.
     */
    transform_.translation = {
        0.0f,
        0.0f,
        0.0f
    };

    transform_.rotation = {
        0.0f,
        0.0f,
        0.0f
    };

    transform_.scale = {
        1.0f,
        1.0f,
        1.0f
    };

    status_ = ModuleStatus::Ready;

    levi::core::Logger::info(
        "ViewModel initialized for Minecraft %s",
        levi::minecraft::MinecraftProfile::kVersion
    );

    return true;
}

void ViewModel::shutdown() noexcept {
    disable();

    transform_.reset();

    status_ = ModuleStatus::Disabled;
}

void ViewModel::tick(
    float deltaTime
) noexcept {
    (void)deltaTime;

    if (!enabled_) {
        return;
    }

    /*
     * The actual MatrixStack mutation happens at the
     * first-person render boundary, not once per tick.
     *
     * This is important:
     *
     * tick()
     *   = update module state
     *
     * render callback
     *   = apply MatrixStack transform
     */
}

bool ViewModel::enable() noexcept {
    if (
        status_ != ModuleStatus::Ready &&
        status_ != ModuleStatus::Active
    ) {
        return false;
    }

    enabled_ = true;

    levi::core::State::instance()
        .setViewModelEnabled(true);

    status_ = ModuleStatus::Active;

    levi::core::Logger::info(
        "ViewModel enabled"
    );

    return true;
}

void ViewModel::disable() noexcept {
    enabled_ = false;

    levi::core::State::instance()
        .setViewModelEnabled(false);

    if (status_ == ModuleStatus::Active) {
        status_ = ModuleStatus::Ready;
    }
}

bool ViewModel::enabled() const noexcept {
    return enabled_;
}

ModuleStatus ViewModel::status() const noexcept {
    return status_;
}

} // namespace levi::modules
