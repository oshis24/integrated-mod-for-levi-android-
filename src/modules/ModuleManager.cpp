#include "levi/modules/ModuleManager.hpp"

#include "levi/minecraft/Camera.hpp"
#include "levi/minecraft/ItemRenderer.hpp"
#include "levi/minecraft/MinecraftProfile.hpp"

#include "levi/modules/Freelook.hpp"
#include "levi/modules/ItemPhysics.hpp"
#include "levi/modules/ViewModel.hpp"

namespace levi::modules {

namespace {

ViewModel g_viewModel;
Freelook g_freelook;
ItemPhysics g_itemPhysics;

minecraft::MinecraftTargets
    g_targets{};

bool g_targetsResolved{false};

std::size_t moduleIndex(
    ModuleId id
) noexcept {
    switch (id) {
        case ModuleId::ViewModel:
            return 0;

        case ModuleId::Freelook:
            return 1;

        case ModuleId::ItemPhysics:
            return 2;
    }

    return 0;
}

bool tryResolveMinecraft() noexcept {
    minecraft::MinecraftTargets
        targets{};

    if (
        !minecraft::MinecraftProfile::
            resolve(targets)
    ) {
        return false;
    }

    g_targets = targets;

    if (
        targets.renderItem != 0 &&
        !minecraft::ItemRenderer::
            attached()
    ) {
        minecraft::ItemRenderer::attach(
            targets.renderItem
        );
    }

    minecraft::Camera::attach(
        targets.getPerspective,
        targets.localPlayerApplyTurnDelta
    );

    g_viewModel.bindNativeTarget(
        targets.getFov
    );

    g_itemPhysics.bindNativeTargets(
        targets.setupAndRender,
        targets.renderItemGroup
    );

    g_targetsResolved =
        targets.renderItem != 0 &&
        targets.getPerspective != 0 &&
        targets.localPlayerApplyTurnDelta != 0 &&
        targets.renderItemGroup != 0;

    return targets.any();
}

} // namespace

ModuleManager::ModuleManager()
    : modules_{
        &g_viewModel,
        &g_freelook,
        &g_itemPhysics
    } {
}

ModuleManager&
ModuleManager::instance() {
    static ModuleManager manager;

    return manager;
}

bool ModuleManager::initialize() noexcept {
    if (initialized_) {
        return true;
    }

    bool success = true;

    for (Module* module : modules_) {
        if (module == nullptr) {
            success = false;
            continue;
        }

        const bool result =
            module->initialize();

        if (
            !result &&
            module->status() !=
                ModuleStatus::
                    WaitingForRuntime &&
            module->status() !=
                ModuleStatus::
                    WaitingForTarget
        ) {
            success = false;
        }
    }

    tryResolveMinecraft();

    initialized_ = success;

    return success;
}

void ModuleManager::shutdown() noexcept {
    if (!initialized_) {
        return;
    }

    g_viewModel.disable();
    g_itemPhysics.disable();
    g_freelook.disable();

    for (Module* module : modules_) {
        if (module != nullptr) {
            module->shutdown();
        }
    }

    minecraft::Camera::detach();
    minecraft::ItemRenderer::detach();

    g_targets = {};
    g_targetsResolved = false;
    initialized_ = false;
}

void ModuleManager::tick(
    float deltaTime
) noexcept {
    if (!initialized_) {
        return;
    }

    if (!g_targetsResolved) {
        tryResolveMinecraft();
    }

    for (Module* module : modules_) {
        if (module != nullptr) {
            module->tick(
                deltaTime
            );
        }
    }
}

Module* ModuleManager::get(
    ModuleId id
) noexcept {
    const auto index =
        moduleIndex(id);

    if (index >= modules_.size()) {
        return nullptr;
    }

    return modules_[index];
}

const Module*
ModuleManager::get(
    ModuleId id
) const noexcept {
    const auto index =
        moduleIndex(id);

    if (index >= modules_.size()) {
        return nullptr;
    }

    return modules_[index];
}

bool ModuleManager::enable(
    ModuleId id
) noexcept {
    Module* module = get(id);

    if (module == nullptr) {
        return false;
    }

    return module->enable();
}

void ModuleManager::disable(
    ModuleId id
) noexcept {
    Module* module = get(id);

    if (module != nullptr) {
        module->disable();
    }
}

bool ModuleManager::initialized()
    const noexcept {
    return initialized_;
}

} // namespace levi::modules
