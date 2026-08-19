#pragma once

#include <cstdint>

namespace levi::memory {

enum class HookStatus {
    Uninitialized,
    Installed,
    Removed,
    Failed
};

class Hook final {
public:
    Hook() = default;

    Hook(
        std::uintptr_t target,
        void* replacement
    ) noexcept;

    ~Hook();

    Hook(const Hook&) = delete;
    Hook& operator=(const Hook&) = delete;

    Hook(Hook&& other) noexcept;
    Hook& operator=(Hook&& other) noexcept;

    bool install() noexcept;

    bool remove() noexcept;

    bool installed() const noexcept;

    HookStatus status() const noexcept;

    std::uintptr_t target() const noexcept;

    void* replacement() const noexcept;

    void* original() const noexcept;

private:
    std::uintptr_t target_{0};
    void* replacement_{nullptr};
    void* original_{nullptr};

    HookStatus status_{
        HookStatus::Uninitialized
    };
};

} // namespace levi::memory
