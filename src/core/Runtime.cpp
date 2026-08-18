#include "levi/core/Runtime.hpp"

#include "levi/core/Logger.hpp"
#include "levi/core/State.hpp"
#include "levi/minecraft/MinecraftProfile.hpp"

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

    initialized_ = true;

    State::instance()
        .setInitialized(true);

    /*
     * We don't resolve before Minecraft's native library
     * exists in the process.
     *
     * The profile can therefore be retried on later ticks.
     */
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

    State::instance()
        .setShuttingDown(true);

    initialized_ = false;

    State::instance()
        .setInitialized(false);

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
    if (
        libraryName == nullptr ||
        libraryName[0] == '\0'
    ) {
        return 0;
    }

    levi::memory::MemoryRange range;

    if (
        levi::memory::PatternScanner::findTextRange(
            libraryName,
            range
        )
    ) {
        return range.start;
    }

    return 0;
}

const std::string&
Runtime::minecraftVersion() const noexcept {
    return minecraftVersion_;
}

bool Runtime::isSupportedMinecraftVersion()
    const noexcept {
    return minecraftVersion_ ==
        "1.26.44.3";
}

} // namespace levi::core
