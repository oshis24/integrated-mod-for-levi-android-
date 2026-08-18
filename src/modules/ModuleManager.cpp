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

minecraft::MinecraftTargets g_targets{};

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
    if (g_profileResolved) {
        return;
    }

    minecraft::MinecraftTargets targets{};

    if (
        !minecraft::MinecraftProfile::resolve(
            targets
        )
    ) {
        return;
    }

    /*
     * Keep the complete resolution result.
     *
     * ItemPhysics consumes setupAndRender and
     * renderItemGroup.
     *
     * ViewModel consumes ItemInHandRenderer.
     */
    g_targets = targets;

    if (
        targets.itemInHandRenderer != 0
    ) {
        /*
         * IMPORTANT:
         *
         * ItemRenderer::attach() currently expects the
         * native renderer OBJECT, while our RE currently
         * gives us the FUNCTION target.
         *
         * Therefore we deliberately do NOT pass the
         * function address as if it were an object pointer.
         *
         * The target is retained for the next native bridge
         * stage.
         */
        levi::core::Logger::info(
            "ItemInHandRenderer function target resolved: %p",
            reinterpret_cast<void*>(
                targets.itemInHandRenderer
            )
        );
    }

    if (
        targets.setupAndRender != 0
    ) {
        g_itemPhysics.bindNativeTargets(
            targets.setupAndRender,
            targets.renderItemGroup
        );

        levi::core::Logger::info(
            "ItemPhysics setupAndRender target=%p",
            reinterpret_cast<void*>(
                targets.setupAndRender
            )
        );
    }

    if (
        targets.renderItemGroup != 0
    ) {
        levi::core::Logger::info(
            "ItemPhysics renderItemGroup target=%p",
            reinterpret_cast<void*>(
                targets.renderItemGroup
            )
        );
    }

    /*
     * We consider target resolution complete only after
     * the profile itself has successfully resolved.
     *
     * A missing optional target does not invalidate the
     * entire profile.
     */
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

bool ModuleManager::initialize() noexcept {
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
     * Minecraft may not be loaded yet.
     *
     * tryResolveMinecraft() is therefore also called
     * from tick().
     */
    tryResolveMinecraft();

    initialized_ = success;

    return success;
}

void ModuleManager::shutdown() noexcept {
    if (!initialized_) {
        return;
    }

    /*
     * Disable modules before destroying their native
     * relationships.
     */
    g_viewModel.disable();
    g_itemPhysics.disable();
    g_freelook.disable();

    /*
     * Native renderer hook must be detached before the
     * target object/function becomes invalid.
     *
     * At this stage ItemRenderer only detaches if an actual
     * hook was installed.
     */
    minecraft::ItemRenderer::detach();

    for (Module* module : modules_) {
        if (module != nullptr) {
            module->shutdown();
        }
    }

    g_targets = {};
    g_profileResolved = false;
    initialized_ = false;

    levi::core::Logger::info(
        "Module manager shutdown complete"
    );
}

void ModuleManager::tick(
    float deltaTime
) noexcept {
    if (!initialized_) {
        return;
    }

    /*
     * Minecraft can finish loading after Levi starts.
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

    return
        module != nullptr &&
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

bool ModuleManager::initialized() const noexcept {
    return initialized_;
}

} // namespace levi::modules
