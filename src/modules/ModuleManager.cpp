#include "levi/modules/ModuleManager.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"

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

bool g_profileResolved = false;

minecraft::MinecraftTargets
    g_targets{};

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

void tryResolveMinecraft() noexcept {
    minecraft::MinecraftTargets targets{};

    if (
        !minecraft::MinecraftProfile::resolve(
            targets
        )
    ) {
        return;
    }

    g_targets =
        targets;

    /*
     * ViewModel:
     *
     * The RenderItem signature is now ABI-correct and the
     * native PL hook backend is available when LeviLauncher
     * exposes the same PL symbols as the working reference.
     */
    if (
        targets.renderItem != 0 &&
        !minecraft::ItemRenderer::attached()
    ) {
        if (
            minecraft::ItemRenderer::attach(
                targets.renderItem
            )
        ) {
            core::Logger::info(
                "ViewModel: RenderItem native hook active"
            );
        }
    }

    /*
     * ItemPhysics:
     *
     * Keep the two independently discovered world-item
     * boundaries available to the module.
     */
    g_itemPhysics.bindNativeTargets(
        targets.setupAndRender,
        targets.renderItemGroup
    );

    /*
     * Mark profile resolved only after the profile itself has
     * produced at least one target.
     */
    if (targets.any()) {
        g_profileResolved = true;
    }
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

    core::Logger::info(
        "ModuleManager initializing"
    );

    bool success = true;

    for (Module* module : modules_) {
        if (module == nullptr) {
            success = false;
            continue;
        }

        if (!module->initialize()) {
            if (
                module->status() !=
                    ModuleStatus::WaitingForRuntime &&
                module->status() !=
                    ModuleStatus::WaitingForTarget
            ) {
                success = false;
            }
        }
    }

    tryResolveMinecraft();

    initialized_ =
        success;

    return success;
}

void ModuleManager::shutdown() noexcept {
    if (!initialized_) {
        return;
    }

    g_viewModel.disable();
    g_itemPhysics.disable();
    g_freelook.disable();

    minecraft::ItemRenderer::detach();

    for (Module* module : modules_) {
        if (module != nullptr) {
            module->shutdown();
        }
    }

    g_targets =
        {};

    g_profileResolved =
        false;

    initialized_ =
        false;
}

void ModuleManager::tick(
    float deltaTime
) noexcept {
    if (!initialized_) {
        return;
    }

    /*
     * Keep trying while libminecraftpe.so is loading.
     */
    if (!g_profileResolved) {
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

    if (
        index >= modules_.size()
    ) {
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

    if (
        index >= modules_.size()
    ) {
        return nullptr;
    }

    return modules_[index];
}

bool ModuleManager::enable(
    ModuleId id
) noexcept {
    Module* module =
        get(id);

    return
        module != nullptr &&
        module->enable();
}

void ModuleManager::disable(
    ModuleId id
) noexcept {
    Module* module =
        get(id);

    if (module != nullptr) {
        module->disable();
    }
}

bool ModuleManager::initialized()
    const noexcept {
    return initialized_;
}

} // namespace levi::modules
