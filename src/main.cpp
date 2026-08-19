#include "levi/core/Logger.hpp"
#include "levi/core/Runtime.hpp"
#include "levi/modules/ModuleManager.hpp"

#include <atomic>
#include <pthread.h>
#include <unistd.h>

namespace {

std::atomic<bool> g_running{false};

pthread_t g_bootstrapThread{};
bool g_threadStarted{false};

void* bootstrapThread(
    void*
) {
    auto& runtime =
        levi::core::Runtime::instance();

    if (!runtime.initialize()) {
        return nullptr;
    }

    auto& modules =
        levi::modules::ModuleManager::
            instance();

    /*
     * preload-native can execute before the final
     * libminecraftpe.so executable mapping is available.
     */
    for (
        int attempt = 0;
        attempt < 80 &&
        g_running.load(
            std::memory_order_acquire
        );
        ++attempt
    ) {
        if (!modules.initialized()) {
            modules.initialize();
        }

        modules.tick(0.0f);

        const bool viewModelReady =
            modules.enable(
                levi::modules::ModuleId::
                    ViewModel
            );

        const bool itemPhysicsReady =
            modules.enable(
                levi::modules::ModuleId::
                    ItemPhysics
            );

        if (
            viewModelReady &&
            itemPhysicsReady
        ) {
            levi::core::Logger::info(
                "LeviModules render hooks active"
            );

            break;
        }

        usleep(250000);
    }

    /*
     * Do not auto-enable Freelook yet.
     *
     * Turn suppression is already implemented, but without
     * the final visual camera-rotation target enabling it
     * would intentionally stop body rotation while the
     * visible camera still does not receive yaw/pitch.
     */

    return nullptr;
}

__attribute__((constructor))
void leviModulesLoad() {
    g_running.store(
        true,
        std::memory_order_release
    );

    if (
        pthread_create(
            &g_bootstrapThread,
            nullptr,
            &bootstrapThread,
            nullptr
        ) == 0
    ) {
        g_threadStarted = true;
    }
}

__attribute__((destructor))
void leviModulesUnload() {
    g_running.store(
        false,
        std::memory_order_release
    );

    if (g_threadStarted) {
        pthread_join(
            g_bootstrapThread,
            nullptr
        );

        g_threadStarted = false;
    }

    levi::modules::ModuleManager::
        instance().shutdown();

    levi::core::Runtime::
        instance().shutdown();
}

} // namespace
