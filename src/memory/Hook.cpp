#include "levi/memory/Hook.hpp"

#include "levi/core/Logger.hpp"

#include <dlfcn.h>

namespace levi::memory {

namespace {

/*
 * Exact mangled PL symbols observed in BetterViewModel:
 *
 * pl::memory::hook(
 *      void*,
 *      void*,
 *      void**,
 *      HookPriority
 * )
 *
 * pl::memory::unhook(
 *      void*,
 *      void*
 * )
 *
 * BetterViewModel passes priority value 200.
 *
 * We intentionally represent HookPriority as int at ABI level.
 */

using PlHookFn = int (*)(
    void*,
    void*,
    void**,
    int
);

using PlUnhookFn = int (*)(
    void*,
    void*
);

constexpr const char* kHookSymbol =
    "_ZN2pl6memory4hookEPvS1_PS1_NS0_12HookPriorityE";

constexpr const char* kUnhookSymbol =
    "_ZN2pl6memory6unhookEPvS1_";

PlHookFn resolveHook() noexcept {
    return reinterpret_cast<PlHookFn>(
        dlsym(
            RTLD_DEFAULT,
            kHookSymbol
        )
    );
}

PlUnhookFn resolveUnhook() noexcept {
    return reinterpret_cast<PlUnhookFn>(
        dlsym(
            RTLD_DEFAULT,
            kUnhookSymbol
        )
    );
}

} // namespace

Hook::Hook(
    std::uintptr_t target,
    void* replacement
) noexcept
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
      status_(other.status_) {

    other.target_ = 0;
    other.replacement_ = nullptr;
    other.original_ = nullptr;
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
    status_ = other.status_;

    other.target_ = 0;
    other.replacement_ = nullptr;
    other.original_ = nullptr;
    other.status_ =
        HookStatus::Uninitialized;

    return *this;
}

bool Hook::install() noexcept {
    if (installed()) {
        return true;
    }

    if (
        target_ == 0 ||
        replacement_ == nullptr
    ) {
        status_ =
            HookStatus::Failed;

        return false;
    }

    const auto hook =
        resolveHook();

    if (hook == nullptr) {
        core::Logger::error(
            "Hook: PL hook backend unavailable"
        );

        status_ =
            HookStatus::Failed;

        return false;
    }

    void* original = nullptr;

    /*
     * BetterViewModel uses priority 200.
     */
    const int priority = 200;

    const int result =
        hook(
            reinterpret_cast<void*>(
                target_
            ),
            replacement_,
            &original,
            priority
        );

    if (
        result != 0 ||
        original == nullptr
    ) {
        core::Logger::error(
            "Hook: PL installation failed "
            "target=%p result=%d",
            reinterpret_cast<void*>(
                target_
            ),
            result
        );

        status_ =
            HookStatus::Failed;

        return false;
    }

    original_ = original;

    status_ =
        HookStatus::Installed;

    core::Logger::info(
        "Hook installed target=%p original=%p",
        reinterpret_cast<void*>(
            target_
        ),
        original_
    );

    return true;
}

bool Hook::remove() noexcept {
    if (!installed()) {
        return true;
    }

    const auto unhook =
        resolveUnhook();

    if (unhook == nullptr) {
        core::Logger::error(
            "Hook: PL unhook backend unavailable"
        );

        return false;
    }

    const int result =
        unhook(
            reinterpret_cast<void*>(
                target_
            ),
            replacement_
        );

    if (result != 0) {
        core::Logger::error(
            "Hook removal failed target=%p result=%d",
            reinterpret_cast<void*>(
                target_
            ),
            result
        );

        return false;
    }

    original_ = nullptr;

    status_ =
        HookStatus::Removed;

    return true;
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
