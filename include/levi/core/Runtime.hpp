#pragma once

#include <cstdint>
#include <string>

namespace levi::core {

class Runtime final {
public:
    static Runtime& instance();

    bool initialize();
    void shutdown();

    bool isInitialized() const noexcept;

    /*
     * Returns the base address of a loaded shared library.
     *
     * Example:
     *     Runtime::instance().findLibrary("libminecraftpe.so");
     *
     * Returns 0 when the library cannot be found.
     */
    std::uintptr_t findLibrary(const char* libraryName) const noexcept;

    /*
     * Minecraft target version selected by the runtime.
     */
    const std::string& minecraftVersion() const noexcept;

    /*
     * Currently supported target:
     *
     *     Minecraft Bedrock 1.26.44.3
     *
     * This is deliberately kept separate from addresses/signatures.
     */
    bool isSupportedMinecraftVersion() const noexcept;

private:
    Runtime() = default;

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

private:
    bool initialized_{false};
    std::string minecraftVersion_{"1.26.44.3"};
};

} // namespace levi::core
