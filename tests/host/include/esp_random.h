#pragma once

#include <cstddef>
#include <cstdint>

/// Host-test stand-in for ESP-IDF's esp_fill_random.
inline void esp_fill_random(
    void* destination,
    std::size_t byteCount
) {
    auto* bytes = static_cast<std::uint8_t*>(destination);

    for (std::size_t index = 0U; index < byteCount; ++index) {
        bytes[index] = static_cast<std::uint8_t>(0xA5U ^ static_cast<std::uint8_t>(index));
    }
}
