#include "levi/core/Runtime.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/State.hpp"

#include <cstdio>
#include <cstring>

#include <dlfcn.h>

namespace levi::core {

Runtime& Runtime::instance() {
    static Runtime runtime;
    return runtime;
}

bool Runtime::initialize() {
    if (initialized_) {
        return true;
    }

    Logger::initialize();

    Logger::info(
        "Initializing LeviModules runtime"
    );

    Logger::info(
        "Target Minecraft version: %s",
        minecraftVersion_.c_str()
    );

    /*
     * Do not resolve Minecraft symbols here yet.
     *
     * Native target resolution will be handled by the
     * memory/profile layer after the library is loaded.
     */

    initialized_ = true;

    State::instance().setInitialized(true);

    Logger::info(
        "Runtime initialized"
    );

    return true;
}

void Runtime::shutdown() {
    if (!initialized_) {
        return;
    }

    Logger::info(
        "Shutting down LeviModules runtime"
    );

    State::instance().setShuttingDown(true);

    /*
     * Hook removal will be implemented by HookManager.
     */

    initialized_ = false;

    State::instance().setInitialized(false);

    Logger::info(
        "Runtime shutdown complete"
    );
}

bool Runtime::isInitialized() const noexcept {
    return initialized_;
}

std::uintptr_t Runtime::findLibrary(
    const char* libraryName
) const noexcept {

    if (libraryName == nullptr || libraryName[0] == '\0') {
        return 0;
    }

    void* handle = dlopen(
        libraryName,
        RTLD_NOW | RTLD_NOLOAD
    );

    if (handle == nullptr) {
        return 0;
    }

    /*
     * dlsym(handle, nullptr) is not portable for retrieving
     * the module base. The actual ELF base resolver will be
     * implemented by PatternScanner using /proc/self/maps.
     */

    dlclose(handle);

    return 0;
}

const std::string& Runtime::minecraftVersion() const noexcept {
    return minecraftVersion_;
}

bool Runtime::isSupportedMinecraftVersion() const noexcept {
    return minecraftVersion_ == "1.26.44.3";
}

} // namespace levi::core
