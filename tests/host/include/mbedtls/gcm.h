#pragma once

#include <cstddef>
#include <cstdint>

#define MBEDTLS_GCM_ENCRYPT 1
#define MBEDTLS_GCM_DECRYPT 0
#define MBEDTLS_CIPHER_ID_AES 1

struct mbedtls_gcm_context {
    std::uint8_t Key[32U]{};
    std::uint8_t Iv[32U]{};
    std::size_t IvBytes = 0U;
    std::uint32_t Hash = 2166136261U;
    int Mode = MBEDTLS_GCM_ENCRYPT;
};

inline void mbedtls_gcm_init(mbedtls_gcm_context* context) {
    *context = mbedtls_gcm_context{};
}

inline int mbedtls_gcm_setkey(
    mbedtls_gcm_context* context,
    int,
    const unsigned char* key,
    unsigned int keyBits
) {
    if (context == nullptr || key == nullptr || keyBits != 256U) return -1;

    for (std::size_t index = 0U; index < 32U; ++index) {
        context->Key[index] = key[index];
    }

    return 0;
}

inline int mbedtls_gcm_starts(
    mbedtls_gcm_context* context,
    int mode,
    const unsigned char* iv,
    std::size_t ivBytes
) {
    if (
        context == nullptr ||
        iv == nullptr ||
        ivBytes == 0U ||
        ivBytes > sizeof(context->Iv)
    ) {
        return -1;
    }

    context->Mode = mode;
    context->IvBytes = ivBytes;

    for (std::size_t index = 0U; index < ivBytes; ++index) {
        context->Iv[index] = iv[index];
        context->Hash ^= iv[index];
        context->Hash *= 16777619U;
    }

    return 0;
}

inline int mbedtls_gcm_update_ad(
    mbedtls_gcm_context* context,
    const unsigned char* data,
    std::size_t dataBytes
) {
    if (context == nullptr || (dataBytes > 0U && data == nullptr)) return -1;

    for (std::size_t index = 0U; index < dataBytes; ++index) {
        context->Hash ^= data[index];
        context->Hash *= 16777619U;
    }

    return 0;
}

inline int mbedtls_gcm_update(
    mbedtls_gcm_context* context,
    const unsigned char* input,
    std::size_t inputBytes,
    unsigned char* output,
    std::size_t outputBytes,
    std::size_t* producedBytes
) {
    if (
        context == nullptr ||
        producedBytes == nullptr ||
        outputBytes < inputBytes ||
        (inputBytes > 0U && (input == nullptr || output == nullptr))
    ) {
        return -1;
    }

    for (std::size_t index = 0U; index < inputBytes; ++index) {
        const auto stream = static_cast<std::uint8_t>(
            context->Key[index % 32U] ^
            context->Iv[index % context->IvBytes]
        );
        output[index] = static_cast<std::uint8_t>(input[index] ^ stream);

        const auto authenticatedByte = context->Mode == MBEDTLS_GCM_ENCRYPT
            ? output[index]
            : input[index];

        context->Hash ^= authenticatedByte;
        context->Hash *= 16777619U;
    }

    *producedBytes = inputBytes;

    return 0;
}

inline int mbedtls_gcm_finish(
    mbedtls_gcm_context* context,
    unsigned char*,
    std::size_t,
    std::size_t* outputBytes,
    unsigned char* tag,
    std::size_t tagBytes
) {
    if (
        context == nullptr ||
        outputBytes == nullptr ||
        tag == nullptr ||
        tagBytes != 16U
    ) {
        return -1;
    }

    *outputBytes = 0U;

    for (std::size_t index = 0U; index < tagBytes; ++index) {
        tag[index] = static_cast<std::uint8_t>(
            context->Hash >> static_cast<unsigned>((index % 4U) * 8U)
        );
    }

    return 0;
}

inline void mbedtls_gcm_free(mbedtls_gcm_context* context) {
    if (context != nullptr) {
        *context = mbedtls_gcm_context{};
    }
}
