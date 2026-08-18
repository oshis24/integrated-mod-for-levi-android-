#include "levi/memory/VTable.hpp"

#include <sys/mman.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>

namespace levi::memory {

namespace {

bool makeWritable(
    void* address,
    std::size_t size
) noexcept {
    if (address == nullptr || size == 0) {
        return false;
    }

    const long pageSize = sysconf(_SC_PAGESIZE);

    if (pageSize <= 0) {
        return false;
    }

    const auto addressValue =
        reinterpret_cast<std::uintptr_t>(address);

    const auto pageMask =
        static_cast<std::uintptr_t>(pageSize - 1);

    const auto pageStart =
        addressValue & ~pageMask;

    const auto pageEnd =
        (addressValue + size + pageMask) & ~pageMask;

    return mprotect(
        reinterpret_cast<void*>(pageStart),
        pageEnd - pageStart,
        PROT_READ | PROT_WRITE
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

    auto*** objectAsTable =
        reinterpret_cast<void***>(object);

    if (*objectAsTable == nullptr) {
        return nullptr;
    }

    return (*objectAsTable)[index];
}

bool VTable::replace(
    void* object,
    std::size_t index,
    void* replacement,
    void** original
) noexcept {

    if (object == nullptr ||
        replacement == nullptr) {
        return false;
    }

    auto*** objectAsTable =
        reinterpret_cast<void***>(object);

    if (*objectAsTable == nullptr) {
        return false;
    }

    void** table = *objectAsTable;

    void* oldFunction = table[index];

    if (original != nullptr) {
        *original = oldFunction;
    }

    /*
     * IMPORTANT:
     *
     * This is intentionally a primitive VTable operation.
     * It is NOT yet the final LeviLaunchroid hook mechanism.
     *
     * For VanillaCameraAPI::vtable[7], we will eventually
     * use the launcher/preloader-safe hook layer if available.
     */

    if (!makeWritable(
            &table[index],
            sizeof(void*))) {
        return false;
    }

    table[index] = replacement;

    __builtin___clear_cache(
        reinterpret_cast<char*>(&table[index]),
        reinterpret_cast<char*>(&table[index]) +
            sizeof(void*)
    );

    return true;
}

} // namespace levi::memory
