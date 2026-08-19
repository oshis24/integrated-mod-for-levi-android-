#include "levi/modules/ViewModel.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"
#include "levi/core/State.hpp"

#include "levi/minecraft/ItemRenderer.hpp"

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

        return false;
    }

    enabled_ = false;

    transform_.reset();

    status_ =
        ModuleStatus::Ready;

    core::Logger::info(
        "ViewModel initialized"
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

    if (!enabled_) {
        return;
    }

    minecraft::ItemRenderer::
        setViewModelTransform(
            transform_
        );
}

bool ViewModel::enable() noexcept {
    if (
        status_ != ModuleStatus::Ready &&
        status_ != ModuleStatus::Active
    ) {
        return false;
    }

    if (
        !minecraft::ItemRenderer::attached()
    ) {
        status_ =
            ModuleStatus::WaitingForTarget;

        core::Logger::warning(
            "ViewModel: RenderItem hook unavailable"
        );

        return false;
    }

    minecraft::ItemRenderer::
        setViewModelTransform(
            transform_
        );

    minecraft::ItemRenderer::
        setViewModelEnabled(
            true
        );

    core::State::instance()
        .setViewModelEnabled(true);

    enabled_ = true;

    status_ =
        ModuleStatus::Active;

    return true;
}

void ViewModel::disable() noexcept {
    enabled_ = false;

    minecraft::ItemRenderer::
        setViewModelEnabled(
            false
        );

    core::State::instance()
        .setViewModelEnabled(false);

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

ModuleStatus
ViewModel::status() const noexcept {
    return status_;
}

} // namespace levi::modules
