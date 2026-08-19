#include "levi/modules/ModuleManager.hpp"

#include "levi/core/Logger.hpp"

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

bool tryResolveMinecraft()
    noexcept {
    minecraft::MinecraftTargets
        targets{};

    if (
        !minecraft::MinecraftProfile::
            resolve(
                targets
            )
    ) {
        return false;
    }

    g_targets =
        targets;

    /*
     * -------------------------------------------------
     * ViewModel
     * -------------------------------------------------
     */

    if (
        targets.renderItem != 0 &&
        !minecraft::ItemRenderer::
            attached()
    ) {
        if (
            minecraft::ItemRenderer::
                attach(
                    targets.renderItem
                )
        ) {
            core::Logger::info(
                "ModuleManager: "
                "RenderItem attached"
            );
        }
    }

    /*
     * Special ItemInHandObject path.
     *
     * Required for chest-family and other special-model
     * first-person items.
     */
    if (
        targets.renderObject != 0 &&
        !minecraft::ItemRenderer::
            renderObjectAttached()
    ) {
        if (
            minecraft::ItemRenderer::
                attachRenderObject(
                    targets.renderObject
                )
        ) {
            core::Logger::info(
                "ModuleManager: "
                "RenderObject attached"
            );
        }
    }

    /*
     * ViewModel item FOV.
     */
    g_viewModel.bindNativeTarget(
        targets.getFov
    );

    /*
     * -------------------------------------------------
     * Camera / Freelook
     * -------------------------------------------------
     *
     * ScreenView::render belongs exclusively to Camera.
     *
     * ItemPhysics no longer hooks it.
     */
    minecraft::Camera::attach(
        targets.getPerspective,
        targets.localPlayerApplyTurnDelta,
        targets.clientInstanceUpdate,
        targets.screenViewRender,
        targets.clientInstanceGetLocalPlayer
    );

    /*
     * -------------------------------------------------
     * ItemPhysics
     * -------------------------------------------------
     */
    g_itemPhysics.bindNativeTarget(
        targets.renderItemGroup
    );

    /*
     * Full supported-build target state.
     *
     * GetFov is optional for basic ViewModel translation,
     * but resolved on the supported build.
     */
    g_targetsResolved =
        targets.renderItem != 0 &&
        targets.renderObject != 0 &&

        targets.getPerspective != 0 &&
        targets.localPlayerApplyTurnDelta != 0 &&
        targets.clientInstanceUpdate != 0 &&
        targets.clientInstanceGetLocalPlayer != 0 &&
        targets.screenViewRender != 0 &&

        targets.renderItemGroup != 0;

    if (g_targetsResolved) {
        core::Logger::info(
            "ModuleManager: "
            "all required native targets resolved"
        );
    }

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

bool ModuleManager::initialize()
    noexcept {
    if (initialized_) {
        return true;
    }

    core::Logger::info(
        "ModuleManager initializing"
    );

    bool success = true;

    for (
        Module* module :
        modules_
    ) {
        if (module == nullptr) {
            success = false;

            continue;
        }

        const bool result =
            module->initialize();

        /*
         * WaitingForRuntime/WaitingForTarget are normal
         * startup states because preload-native can run
         * before Minecraft has finished loading.
         */
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

            core::Logger::error(
                "Module initialization failed: %s",
                module->name()
            );
        }
    }

    /*
     * First native-resolution attempt.
     *
     * tick() will retry if Minecraft is not ready yet.
     */
    tryResolveMinecraft();

    initialized_ =
        success;

    core::Logger::info(
        "ModuleManager initialized"
    );

    return success;
}

void ModuleManager::shutdown()
    noexcept {
    if (!initialized_) {
        return;
    }

    core::Logger::info(
        "ModuleManager shutting down"
    );

    /*
     * Disable modules before removing native hooks.
     */
    g_viewModel.disable();
    g_itemPhysics.disable();
    g_freelook.disable();

    /*
     * Give modules a chance to release their own hooks and
     * callbacks first.
     */
    for (
        Module* module :
        modules_
    ) {
        if (module != nullptr) {
            module->shutdown();
        }
    }

    /*
     * Shared rendering/camera hooks come last.
     */
    minecraft::Camera::detach();

    minecraft::ItemRenderer::
        detach();

    g_targets =
        {};

    g_targetsResolved =
        false;

    initialized_ =
        false;

    core::Logger::info(
        "ModuleManager shutdown complete"
    );
}

void ModuleManager::tick(
    float deltaTime
) noexcept {
    if (!initialized_) {
        return;
    }

    /*
     * Retry resolution because preload-native can execute
     * before libminecraftpe.so has a final executable
     * mapping.
     */
    if (!g_targetsResolved) {
        tryResolveMinecraft();
    }

    for (
        Module* module :
        modules_
    ) {
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
    const std::size_t index =
        moduleIndex(id);

    if (
        index >=
        modules_.size()
    ) {
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

    if (
        index >=
        modules_.size()
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

    if (module == nullptr) {
        return false;
    }

    const bool result =
        module->enable();

    if (result) {
        core::Logger::info(
            "Module enabled: %s",
            module->name()
        );
    } else {
        core::Logger::warning(
            "Module enable pending/failed: %s",
            module->name()
        );
    }

    return result;
}

void ModuleManager::disable(
    ModuleId id
) noexcept {
    Module* module =
        get(id);

    if (module == nullptr) {
        return;
    }

    module->disable();

    core::Logger::info(
        "Module disabled: %s",
        module->name()
    );
}

bool ModuleManager::initialized()
    const noexcept {
    return initialized_;
}

} // namespace levi::modules
