#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#define MALLOC_CAP_8BIT (1U << 0U)
#define MALLOC_CAP_INTERNAL (1U << 1U)
#define MALLOC_CAP_SPIRAM (1U << 2U)

inline bool EspIdfTestFailExternal = false;
inline std::uint32_t EspIdfTestLastCapabilities = 0U;
inline std::size_t EspIdfTestAllocationAttempts = 0U;

/// Host stub matching the ESP-IDF aligned heap-capability allocation surface used by the provider.
inline void* heap_caps_aligned_alloc(
    std::size_t alignment,
    std::size_t size,
    std::uint32_t capabilities
) {
    EspIdfTestLastCapabilities = capabilities;
    ++EspIdfTestAllocationAttempts;

    if (
        EspIdfTestFailExternal &&
        (capabilities & MALLOC_CAP_SPIRAM) != 0U
    ) {
        return nullptr;
    }

    void* allocation = nullptr;

    if (posix_memalign(
        &allocation,
        alignment,
        size
    ) != 0) {
        return nullptr;
    }

    return allocation;
}

/// Host stub for the common ESP-IDF heap release surface.
inline void heap_caps_free(
    void* allocation
) {
    std::free(allocation);
}
