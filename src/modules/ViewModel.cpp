#include "levi/modules/ViewModel.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"
#include "levi/core/State.hpp"
#include "levi/minecraft/Camera.hpp"
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

    active_ = this;

    transform_.reset();

    levi::minecraft::Camera::
        setPerspectiveObserver(
            &ViewModel::onPerspective
        );

    status_ =
        ModuleStatus::Ready;

    return true;
}

void ViewModel::bindNativeTarget(
    std::uintptr_t getFov
) noexcept {
    if (
        getFov == 0 ||
        fovHook_.installed()
    ) {
        return;
    }

    fovHook_ =
        levi::memory::Hook(
            getFov,
            reinterpret_cast<void*>(
                &ViewModel::fovDetour
            )
        );

    if (!fovHook_.install()) {
        levi::core::Logger::warning(
            "ViewModel: GetFov hook unavailable"
        );
    }
}

void ViewModel::shutdown() noexcept {
    disable();

    fovHook_.remove();

    if (active_ == this) {
        active_ = nullptr;
    }

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

    levi::minecraft::ItemRenderer::
        setViewModelTransform(
            transform_
        );

    levi::minecraft::ItemRenderer::
        setViewModelPerspective(
            thirdPerson_,
            applyThirdPerson_
        );
}

bool ViewModel::enable() noexcept {
    if (
        !levi::minecraft::ItemRenderer::
            attached()
    ) {
        status_ =
            ModuleStatus::WaitingForTarget;

        return false;
    }

    enabled_ = true;

    levi::minecraft::ItemRenderer::
        setViewModelEnabled(
            true
        );

    levi::minecraft::ItemRenderer::
        setViewModelTransform(
            transform_
        );

    levi::minecraft::ItemRenderer::
        setViewModelPerspective(
            thirdPerson_,
            applyThirdPerson_
        );

    levi::core::State::instance()
        .setViewModelEnabled(true);

    status_ =
        ModuleStatus::Active;

    return true;
}

void ViewModel::disable() noexcept {
    enabled_ = false;

    levi::minecraft::ItemRenderer::
        setViewModelEnabled(
            false
        );

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

float ViewModel::fovDetour(
    void* self,
    float value,
    int enableVariableFov
) noexcept {
    auto* module = active_;

    const auto original =
        module != nullptr
            ? reinterpret_cast<GetFovFn>(
                module->fovHook_.original()
            )
            : nullptr;

    float result =
        original != nullptr
            ? original(
                self,
                value,
                enableVariableFov
            )
            : 0.0f;

    if (
        module != nullptr &&
        module->enabled_ &&
        result >= 69.5f &&
        result <= 70.5f
    ) {
        result =
            module->itemFov_;
    }

    return result;
}

void ViewModel::onPerspective(
    int perspective
) noexcept {
    auto* module = active_;

    if (module == nullptr) {
        return;
    }

    module->thirdPerson_ =
        perspective != 0;

    levi::minecraft::ItemRenderer::
        setViewModelPerspective(
            module->thirdPerson_,
            module->applyThirdPerson_
        );
}

} // namespace levi::modules
