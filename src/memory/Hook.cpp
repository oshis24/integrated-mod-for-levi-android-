#include "levi/memory/Hook.hpp"

#include "levi/core/Logger.hpp"

#include <new>

/*
 * LeviLauncher build must provide the PL hook backend.
 *
 * BedrockTools' working implementation uses exactly the same
 * interface:
 *
 *     pl::memory::hook(target, detour, original)
 *     pl::memory::unhook(target, detour)
 *
 * Do NOT replace this with an ad-hoc 16-byte ARM64 patch.
 */
#if __has_include(<pl/memory/Hook.hpp>)
    #include <pl/memory/Hook.hpp>
    #define LEVI_HAS_PL_HOOK 1
#else
    #define LEVI_HAS_PL_HOOK 0
#endif

namespace levi::memory {

namespace {

#if LEVI_HAS_PL_HOOK

struct BackendState final {
    void* target{nullptr};
    void* replacement{nullptr};
};

#endif

} // namespace

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

Hook::Hook(
    Hook&& other
) noexcept
    : target_(other.target_),
      replacement_(other.replacement_),
      original_(other.original_),
      backendState_(other.backendState_),
      status_(other.status_) {

    other.target_ = 0;
    other.replacement_ = nullptr;
    other.original_ = nullptr;
    other.backendState_ = nullptr;

    other.status_ =
        HookStatus::Uninitialized;
}

Hook& Hook::operator=(
    Hook&& other
) noexcept {
    if (this == &other) {
        return *this;
    }

    remove();

    target_ = other.target_;
    replacement_ = other.replacement_;
    original_ = other.original_;
    backendState_ = other.backendState_;
    status_ = other.status_;

    other.target_ = 0;
    other.replacement_ = nullptr;
    other.original_ = nullptr;
    other.backendState_ = nullptr;

    other.status_ =
        HookStatus::Uninitialized;

    return *this;
}

bool Hook::install() {
    if (installed()) {
        return true;
    }

    if (
        target_ == 0 ||
        replacement_ == nullptr
    ) {
        status_ =
            HookStatus::Failed;

        core::Logger::error(
            "Hook: invalid target/replacement"
        );

        return false;
    }

#if !LEVI_HAS_PL_HOOK

    core::Logger::error(
        "Hook: ARM64 backend unavailable; "
        "pl/memory/Hook.hpp not provided"
    );

    status_ =
        HookStatus::Failed;

    return false;

#else

    void* original = nullptr;

    /*
     * PL performs the actual ARM64 trampoline generation
     * and instruction relocation.
     */
    const int result =
        pl::memory::hook(
            reinterpret_cast<void*>(target_),
            replacement_,
            &original
        );

    if (result != 0 || original == nullptr) {
        core::Logger::error(
            "Hook: backend installation failed "
            "target=%p result=%d",
            reinterpret_cast<void*>(target_),
            result
        );

        status_ =
            HookStatus::Failed;

        return false;
    }

    auto* state =
        new (std::nothrow) BackendState{
            reinterpret_cast<void*>(target_),
            replacement_
        };

    if (state == nullptr) {
        /*
         * Roll back immediately if we cannot retain the
         * backend state.
         */
        pl::memory::unhook(
            reinterpret_cast<void*>(target_),
            replacement_
        );

        status_ =
            HookStatus::Failed;

        return false;
    }

    original_ = original;
    backendState_ = state;

    status_ =
        HookStatus::Installed;

    core::Logger::info(
        "Hook installed target=%p replacement=%p "
        "original=%p",
        reinterpret_cast<void*>(target_),
        replacement_,
        original_
    );

    return true;

#endif
}

bool Hook::remove() {
    if (!installed()) {
        return true;
    }

#if !LEVI_HAS_PL_HOOK

    status_ =
        HookStatus::Removed;

    original_ = nullptr;
    backendState_ = nullptr;

    return true;

#else

    if (
        target_ == 0 ||
        replacement_ == nullptr
    ) {
        status_ =
            HookStatus::Removed;

        original_ = nullptr;
        backendState_ = nullptr;

        return true;
    }

    const int result =
        pl::memory::unhook(
            reinterpret_cast<void*>(target_),
            replacement_
        );

    if (result != 0) {
        core::Logger::error(
            "Hook removal failed target=%p result=%d",
            reinterpret_cast<void*>(target_),
            result
        );

        return false;
    }

    delete static_cast<BackendState*>(
        backendState_
    );

    backendState_ = nullptr;
    original_ = nullptr;

    status_ =
        HookStatus::Removed;

    return true;

#endif
}

bool Hook::installed() const noexcept {
    return status_ ==
        HookStatus::Installed;
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
