#pragma once

#include <cstddef>

#include <openssl/evp.h>

struct mbedtls_sha256_context final {
    EVP_MD_CTX* Context = nullptr;
};

inline bool mbedtls_test_sha256_fail_next_operation = false;

inline bool mbedtls_test_sha256_should_fail() noexcept {
    if (!mbedtls_test_sha256_fail_next_operation) return false;

    mbedtls_test_sha256_fail_next_operation = false;
    return true;
}

inline void mbedtls_sha256_init(
    mbedtls_sha256_context* context
) noexcept {
    context->Context = EVP_MD_CTX_new();
}

inline void mbedtls_sha256_free(
    mbedtls_sha256_context* context
) noexcept {
    EVP_MD_CTX_free(context->Context);
    context->Context = nullptr;
}

inline int mbedtls_sha256_starts(
    mbedtls_sha256_context* context,
    int is224
) noexcept {
    if (
        mbedtls_test_sha256_should_fail() ||
        context->Context == nullptr ||
        is224 != 0
    ) {
        return -1;
    }

    return EVP_DigestInit_ex(
        context->Context,
        EVP_sha256(),
        nullptr
    ) == 1 ? 0 : -1;
}

inline int mbedtls_sha256_update(
    mbedtls_sha256_context* context,
    const unsigned char* input,
    std::size_t inputLength
) noexcept {
    if (
        mbedtls_test_sha256_should_fail() ||
        context->Context == nullptr ||
        (inputLength > 0U && input == nullptr)
    ) {
        return -1;
    }

    if (inputLength == 0U) return 0;

    return EVP_DigestUpdate(
        context->Context,
        input,
        inputLength
    ) == 1 ? 0 : -1;
}

inline int mbedtls_sha256_finish(
    mbedtls_sha256_context* context,
    unsigned char output[32U]
) noexcept {
    if (
        mbedtls_test_sha256_should_fail() ||
        context->Context == nullptr ||
        output == nullptr
    ) {
        return -1;
    }

    unsigned int outputLength = 0U;
    return
        EVP_DigestFinal_ex(
            context->Context,
            output,
            &outputLength
        ) == 1 &&
        outputLength == 32U
            ? 0
            : -1;
}
