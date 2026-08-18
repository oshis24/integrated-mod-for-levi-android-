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

void tryResolveMinecraft()
    noexcept {
    if (g_profileResolved) {
        return;
    }

    levi::minecraft::MinecraftProfile::Resolution
        resolution;

    if (
        !levi::minecraft::MinecraftProfile::resolve(
            resolution
        )
    ) {
        return;
    }

    /*
     * The profile must eventually provide the native
     * ItemInHandRenderer object acquisition path.
     *
     * At the current repository state we do NOT possess
     * that object pointer yet, so don't fabricate one.
     */
    if (
        resolution.render.renderFirstPerson.valid()
    ) {
        levi::core::Logger::info(
            "Native renderFirstPerson target resolved"
        );
    }

    if (
        resolution.camera.cameraVFunc.valid()
    ) {
        levi::core::Logger::info(
            "Native camera target resolved"
        );
    }

    g_profileResolved = true;
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

bool ModuleManager::initialize()
    noexcept {
    if (initialized_) {
        return true;
    }

    levi::core::Logger::info(
        "Initializing module manager"
    );

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
                ModuleStatus::WaitingForRuntime &&
            module->status() !=
                ModuleStatus::WaitingForTarget
        ) {
            success = false;

            levi::core::Logger::error(
                "Module initialization failed: %s",
                module->name()
            );
        }
    }

    /*
     * Do this after module initialization.
     *
     * Native library loading may occur after Levi itself.
     */
    tryResolveMinecraft();

    initialized_ = success;

    return success;
}

void ModuleManager::shutdown()
    noexcept {
    if (!initialized_) {
        return;
    }

    /*
     * ViewModel must be disabled before the native
     * renderer object can disappear.
     */
    for (Module* module : modules_) {
        if (module != nullptr) {
            module->shutdown();
        }
    }

    initialized_ = false;
    g_profileResolved = false;
}

void ModuleManager::tick(
    float deltaTime
) noexcept {
    if (!initialized_) {
        return;
    }

    /*
     * Retry native resolution while Minecraft is loading.
     */
    if (!g_profileResolved) {
        tryResolveMinecraft();
    }

    for (Module* module : modules_) {
        if (module != nullptr) {
            module->tick(deltaTime);
        }
    }
}

Module* ModuleManager::get(
    ModuleId id
) noexcept {
    const std::size_t index =
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
    const std::size_t index =
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

    return module != nullptr &&
           module->enable();
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
