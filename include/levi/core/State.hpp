#pragma once

#include <atomic>

namespace levi::core {

class State final {
public:
    static State& instance();

    bool initialized() const noexcept;
    void setInitialized(bool value) noexcept;

    bool shuttingDown() const noexcept;
    void setShuttingDown(bool value) noexcept;

    bool viewModelEnabled() const noexcept;
    void setViewModelEnabled(bool value) noexcept;

    bool freelookEnabled() const noexcept;
    void setFreelookEnabled(bool value) noexcept;

    bool itemPhysicsEnabled() const noexcept;
    void setItemPhysicsEnabled(bool value) noexcept;

private:
    State() = default;

    State(const State&) = delete;
    State& operator=(const State&) = delete;

private:
    std::atomic_bool initialized_{false};
    std::atomic_bool shuttingDown_{false};

    std::atomic_bool viewModelEnabled_{true};
    std::atomic_bool freelookEnabled_{true};
    std::atomic_bool itemPhysicsEnabled_{true};
};

} // namespace levi::core
