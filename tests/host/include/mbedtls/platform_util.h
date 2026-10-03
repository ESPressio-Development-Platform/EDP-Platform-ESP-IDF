#pragma once

#include <cstddef>
#include <cstdint>

inline std::size_t mbedtls_test_zeroize_call_count = 0U;
inline std::size_t mbedtls_test_zeroize_byte_count = 0U;

inline void mbedtls_test_reset_zeroize_evidence() noexcept {
    mbedtls_test_zeroize_call_count = 0U;
    mbedtls_test_zeroize_byte_count = 0U;
}

inline void mbedtls_platform_zeroize(
    void* buffer,
    std::size_t size
) noexcept {
    ++mbedtls_test_zeroize_call_count;
    mbedtls_test_zeroize_byte_count += size;

    volatile auto* output = static_cast<volatile std::uint8_t*>(buffer);
    for (std::size_t index = 0U; index < size; ++index) {
        output[index] = 0U;
    }
}
