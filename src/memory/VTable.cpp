#include "levi/memory/VTable.hpp"

#include <sys/mman.h>
#include <unistd.h>

#include <cstdint>

namespace levi::memory {

namespace {

bool changeProtection(
    void* address,
    std::size_t size,
    int protection
) noexcept {
    if (address == nullptr || size == 0) {
        return false;
    }

    const long pageSize =
        sysconf(_SC_PAGESIZE);

    if (pageSize <= 0) {
        return false;
    }

    const auto page =
        static_cast<std::uintptr_t>(
            pageSize
        );

    const auto value =
        reinterpret_cast<std::uintptr_t>(
            address
        );

    const auto begin =
        value & ~(page - 1);

    const auto end =
        (value + size + page - 1) &
        ~(page - 1);

    return mprotect(
        reinterpret_cast<void*>(begin),
        end - begin,
        protection
    ) == 0;
}

} // namespace

void* VTable::get(
    void* object,
    std::size_t index
) noexcept {
    if (object == nullptr) {
        return nullptr;
    }

    auto*** table =
        reinterpret_cast<void***>(object);

    if (table == nullptr || *table == nullptr) {
        return nullptr;
    }

    return (*table)[index];
}

bool VTable::replace(
    void* object,
    std::size_t index,
    void* replacement,
    void** original
) noexcept {
    if (
        object == nullptr ||
        replacement == nullptr
    ) {
        return false;
    }

    auto*** objectTable =
        reinterpret_cast<void***>(object);

    if (
        objectTable == nullptr ||
        *objectTable == nullptr
    ) {
        return false;
    }

    void** table = *objectTable;

    void* old = table[index];

    if (old == nullptr) {
        return false;
    }

    if (original != nullptr) {
        *original = old;
    }

    if (!changeProtection(
            &table[index],
            sizeof(void*),
            PROT_READ | PROT_WRITE
        )) {
        return false;
    }

    table[index] = replacement;

    __builtin___clear_cache(
        reinterpret_cast<char*>(&table[index]),
        reinterpret_cast<char*>(&table[index]) +
            sizeof(void*)
    );

    /*
     * Restore read-only protection.
     *
     * This is important because leaving Minecraft's vtable
     * writable is unnecessary and dangerous.
     */
    changeProtection(
        &table[index],
        sizeof(void*),
        PROT_READ
    );

    return true;
}

bool VTable::restore(
    void* object,
    std::size_t index,
    void* original
) noexcept {
    if (
        object == nullptr ||
        original == nullptr
    ) {
        return false;
    }

    auto*** objectTable =
        reinterpret_cast<void***>(object);

    if (
        objectTable == nullptr ||
        *objectTable == nullptr
    ) {
        return false;
    }

    void** table = *objectTable;

    if (!changeProtection(
            &table[index],
            sizeof(void*),
            PROT_READ | PROT_WRITE
        )) {
        return false;
    }

    table[index] = original;

    __builtin___clear_cache(
        reinterpret_cast<char*>(&table[index]),
        reinterpret_cast<char*>(&table[index]) +
            sizeof(void*)
    );

    changeProtection(
        &table[index],
        sizeof(void*),
        PROT_READ
    );

    return true;
}

} // namespace levi::memory
