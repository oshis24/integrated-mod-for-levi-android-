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

    transform_.reset();

    status_ =
        ModuleStatus::Ready;

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

    /*
     * Synchronize configuration with the render bridge.
     */
    levi::minecraft::ItemRenderer::
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

    /*
     * Do not pretend the native hook exists.
     *
     * Module state can be active only once the native
     * renderer is actually attached.
     */
    if (
        !levi::minecraft::ItemRenderer::attached()
    ) {
        status_ =
            ModuleStatus::WaitingForTarget;

        levi::core::Logger::warning(
            "ViewModel: ItemInHandRenderer "
            "is not attached"
        );

        return false;
    }

    enabled_ = true;

    levi::minecraft::ItemRenderer::
        setViewModelTransform(
            transform_
        );

    levi::minecraft::ItemRenderer::
        setViewModelEnabled(true);

    levi::core::State::instance()
        .setViewModelEnabled(true);

    status_ =
        ModuleStatus::Active;

    return true;
}

void ViewModel::disable() noexcept {
    enabled_ = false;

    levi::minecraft::ItemRenderer::
        setViewModelEnabled(false);

    levi::core::State::instance()
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
