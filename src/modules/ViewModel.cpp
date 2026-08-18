#include "levi/modules/ViewModel.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"
#include "levi/core/State.hpp"
#include "levi/minecraft/ItemRenderer.hpp"
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
        status_ =
            ModuleStatus::WaitingForRuntime;

        return false;
    }

    if (
        !runtime.isSupportedMinecraftVersion()
    ) {
        status_ =
            ModuleStatus::Failed;

        levi::core::Logger::error(
            "ViewModel: unsupported Minecraft %s",
            runtime.minecraftVersion().c_str()
        );

        return false;
    }

    transform_.reset();

    status_ =
        ModuleStatus::WaitingForTarget;

    levi::core::Logger::info(
        "ViewModel: waiting for ItemInHandRenderer"
    );

    return true;
}

void ViewModel::shutdown() noexcept {
    disable();

    transform_.reset();

    status_ =
        ModuleStatus::Disabled;
}

void ViewModel::tick(
    float deltaTime
) noexcept {
    (void)deltaTime;

    /*
     * Render-time transform is deliberately NOT applied here.
     *
     * tick()
     *     = state
     *
     * ItemRenderer callback
     *     = MatrixStack mutation
     */
}

bool ViewModel::enable() noexcept {
    if (
        !levi::minecraft::ItemRenderer::attached()
    ) {
        status_ =
            ModuleStatus::WaitingForTarget;

        return false;
    }

    enabled_ = true;

    levi::core::State::instance()
        .setViewModelEnabled(true);

    levi::minecraft::ItemRenderer::
        setViewModelEnabled(true);

    levi::minecraft::ItemRenderer::
        setViewModelTransform(transform_);

    status_ =
        ModuleStatus::Active;

    levi::core::Logger::info(
        "ViewModel enabled"
    );

    return true;
}

void ViewModel::disable() noexcept {
    enabled_ = false;

    levi::core::State::instance()
        .setViewModelEnabled(false);

    levi::minecraft::ItemRenderer::
        setViewModelEnabled(false);

    if (
        status_ == ModuleStatus::Active
    ) {
        status_ =
            ModuleStatus::Ready;
    }
}

bool ViewModel::enabled() const noexcept {
    return enabled_;
}

ModuleStatus ViewModel::status() const noexcept {
    return status_;
}

} // namespace levi::modules
