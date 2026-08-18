#include "levi/memory/Hook.hpp"

#include "levi/core/Logger.hpp"

namespace levi::memory {

Hook::Hook(
    std::uintptr_t target,
    void* replacement
)
    : target_(target),
      replacement_(replacement) {
}

Hook::~Hook() {
    remove();
}

Hook::Hook(Hook&& other) noexcept
    : target_(other.target_),
      replacement_(other.replacement_),
      original_(other.original_),
      status_(other.status_) {

    other.target_ = 0;
    other.replacement_ = nullptr;
    other.original_ = nullptr;
    other.status_ = HookStatus::Uninitialized;
}

Hook& Hook::operator=(Hook&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    remove();

    target_ = other.target_;
    replacement_ = other.replacement_;
    original_ = other.original_;
    status_ = other.status_;

    other.target_ = 0;
    other.replacement_ = nullptr;
    other.original_ = nullptr;
    other.status_ = HookStatus::Uninitialized;

    return *this;
}

bool Hook::install() {
    if (installed()) {
        return true;
    }

    if (target_ == 0 || replacement_ == nullptr) {
        status_ = HookStatus::Failed;

        core::Logger::error(
            "Hook installation failed: invalid target/replacement"
        );

        return false;
    }

    /*
     * IMPORTANT:
     *
     * The actual ARM64 hook backend is intentionally not
     * implemented here yet.
     *
     * We will connect this abstraction to the hook mechanism
     * used/allowed by LeviLaunchroid after its API is confirmed.
     *
     * Never patch arbitrary ARM64 instructions here.
     */

    core::Logger::warning(
        "Hook backend not connected yet: target=%p",
        reinterpret_cast<void*>(target_)
    );

    status_ = HookStatus::Failed;

    return false;
}

bool Hook::remove() {
    if (!installed()) {
        return true;
    }

    /*
     * Backend-specific unhooking will be implemented together
     * with install().
     */

    status_ = HookStatus::Removed;
    original_ = nullptr;

    return true;
}

bool Hook::installed() const noexcept {
    return status_ == HookStatus::Installed;
}

HookStatus Hook::status() const noexcept {
    return status_;
}

std::uintptr_t Hook::target() const noexcept {
    return target_;
}

void* Hook::replacement() const noexcept {
    return replacement_;
}

void* Hook::original() const noexcept {
    return original_;
}

} // namespace levi::memory
