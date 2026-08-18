#pragma once

#include <cstdint>

namespace levi::minecraft {

class ClientInstance final {
public:
    ClientInstance() = default;

    explicit ClientInstance(
        std::uintptr_t address
    )
        : address_(address) {
    }

    bool valid() const noexcept {
        return address_ != 0;
    }

    std::uintptr_t address() const noexcept {
        return address_;
    }

    /*
     * These accessors remain opaque until the exact
     * ClientInstance layout/function targets for 1.26.44.3
     * are verified.
     */

    std::uintptr_t camera() const noexcept {
        return camera_;
    }

    std::uintptr_t localPlayer() const noexcept {
        return localPlayer_;
    }

    void setCamera(
        std::uintptr_t address
    ) noexcept {
        camera_ = address;
    }

    void setLocalPlayer(
        std::uintptr_t address
    ) noexcept {
        localPlayer_ = address;
    }

private:
    std::uintptr_t address_{0};

    /*
     * Runtime-resolved pointers.
     *
     * They are NOT offsets into ClientInstance.
     */
    std::uintptr_t camera_{0};
    std::uintptr_t localPlayer_{0};
};

} // namespace levi::minecraft
