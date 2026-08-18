#include "levi/core/State.hpp"

namespace levi::core {

State& State::instance() {
    static State state;
    return state;
}

bool State::initialized() const noexcept {
    return initialized_.load(std::memory_order_acquire);
}

void State::setInitialized(bool value) noexcept {
    initialized_.store(value, std::memory_order_release);
}

bool State::shuttingDown() const noexcept {
    return shuttingDown_.load(std::memory_order_acquire);
}

void State::setShuttingDown(bool value) noexcept {
    shuttingDown_.store(value, std::memory_order_release);
}

bool State::viewModelEnabled() const noexcept {
    return viewModelEnabled_.load(std::memory_order_relaxed);
}

void State::setViewModelEnabled(bool value) noexcept {
    viewModelEnabled_.store(value, std::memory_order_relaxed);
}

bool State::freelookEnabled() const noexcept {
    return freelookEnabled_.load(std::memory_order_relaxed);
}

void State::setFreelookEnabled(bool value) noexcept {
    freelookEnabled_.store(value, std::memory_order_relaxed);
}

bool State::itemPhysicsEnabled() const noexcept {
    return itemPhysicsEnabled_.load(std::memory_order_relaxed);
}

void State::setItemPhysicsEnabled(bool value) noexcept {
    itemPhysicsEnabled_.store(value, std::memory_order_relaxed);
}

} // namespace levi::core
