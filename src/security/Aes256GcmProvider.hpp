#pragma once

#include <cstddef>
#include <cstdint>

#if defined(ESP_PLATFORM) && !defined(MBEDTLS_CONFIG_FILE)
#define MBEDTLS_CONFIG_FILE "mbedtls/esp_config.h"
#endif

#include <mbedtls/gcm.h>

#include <ESPressio_Memory.hpp>
#include <ESPressio_Security.hpp>

namespace ESPressio::Platform::ESPIDF::Security {

    namespace Framework = ESPressio::System::CompositionFramework;

    namespace Detail {

        /// Internal outcome while driving one Mbed TLS GCM streaming operation.
        enum class Aes256GcmProcessingResult : std::uint8_t {
            Succeeded = 0,
            InvalidInput = 1,
            ProviderFailure = 2
        };

    } // ESPressio::Platform::ESPIDF::Security::Detail


    /// ESP-IDF/Mbed TLS AES-256-GCM provider for the EDP-Security AuthenticatedCrypto capability.
    ///
    /// @tparam TByteOperationsProvider Concrete EDP-Memory ByteOperations provider used for bounded
    ///         tail copies without introducing direct C-library memory calls.
    template<class TByteOperationsProvider>
    class Aes256GcmProvider final : public Framework::Provider<
        ESPressio::Security::Domain,
        Framework::Provides<
            Framework::Offer<
                ESPressio::Security::AuthenticatedCrypto,
                Framework::PropertyValue<
                    ESPressio::Security::AuthenticatedCryptoAlgorithm,
                    ESPressio::Security::AuthenticatedAlgorithm::Aes256Gcm
                >,
                Framework::PropertyValue<
                    ESPressio::Security::AuthenticatedCryptoKeyBytes,
                    32U
                >,
                Framework::PropertyValue<
                    ESPressio::Security::AuthenticatedCryptoDefaultNonceBytes,
                    12U
                >,
                Framework::PropertyValue<
                    ESPressio::Security::AuthenticatedCryptoTagBytes,
                    16U
                >,
                Framework::FlagPropertyValue<
                    ESPressio::Security::AuthenticatedCryptoKeyAccessModes,
                    ESPressio::Security::KeyAccessMode::RawMaterial
                >
            >
        >,
        Framework::Requires<>,
        Framework::DependsOn<
            Framework::Need<ESPressio::Memory::ByteOperations>
        >
    > {

        private:

            // Provider dependencies.

            /// Canonical byte-operation provider selected by Bootstrap.
            const TByteOperationsProvider& _bytes;


            // Provider constants.

            /// AES-256 key bytes.
            static constexpr std::size_t KeyBytes = 32U;

            /// AES-GCM authentication tag bytes.
            static constexpr std::size_t TagBytes = 16U;

            /// Native AES block bytes used for deterministic streaming chunks.
            static constexpr std::size_t BlockBytes = 16U;

            /// Tail scratch bytes sufficient for one partial GCM update plus finish output.
            static constexpr std::size_t TailScratchBytes = 32U;


            // Input validation.

            /// Indicates whether two fixed-size tags are equal without data-dependent early exit.
            static bool IsTagEqual(
                const std::uint8_t* left,
                const std::uint8_t* right
            ) noexcept {
                std::uint8_t difference = 0U;

                for (std::size_t index = 0U; index < TagBytes; ++index) {
                    difference = static_cast<std::uint8_t>(
                        difference |
                        static_cast<std::uint8_t>(left[index] ^ right[index])
                    );
                }

                return difference == 0U;
            }


            // GCM setup.

            /// Sets the AES-256 key, starts GCM and feeds every associated-data segment in order.
            template<class TKeyProvider>
            Detail::Aes256GcmProcessingResult StartOperation(
                mbedtls_gcm_context& context,
                int mode,
                const TKeyProvider& keys,
                const ESPressio::Security::KeySelection& selection,
                ESPressio::Security::ByteView nonce,
                ESPressio::Security::AssociatedDataView associatedData
            ) const noexcept {
                ESPressio::Security::RawKeyMaterialView key;

                if (
                    keys.AccessRawKey(
                        selection,
                        key
                    ) != ESPressio::Security::KeyMaterialAccessResult::Succeeded ||
                    !key.IsValid() ||
                    key.Size != KeyBytes
                ) {
                    return Detail::Aes256GcmProcessingResult::InvalidInput;
                }

                if (
                    !nonce.IsValid() ||
                    nonce.Size == 0U ||
                    !associatedData.IsValid()
                ) {
                    return Detail::Aes256GcmProcessingResult::InvalidInput;
                }

                if (
                    mbedtls_gcm_setkey(
                        &context,
                        MBEDTLS_CIPHER_ID_AES,
                        key.Data,
                        256U
                    ) != 0
                ) {
                    return Detail::Aes256GcmProcessingResult::ProviderFailure;
                }

                if (
                    mbedtls_gcm_starts(
                        &context,
                        mode,
                        nonce.Data,
                        nonce.Size
                    ) != 0
                ) {
                    return Detail::Aes256GcmProcessingResult::ProviderFailure;
                }

                for (std::size_t index = 0U; index < associatedData.Count; ++index) {
                    const auto& segment = associatedData.Segments[index];

                    if (segment.Size == 0U) continue;

                    if (
                        mbedtls_gcm_update_ad(
                            &context,
                            segment.Data,
                            segment.Size
                        ) != 0
                    ) {
                        return Detail::Aes256GcmProcessingResult::ProviderFailure;
                    }
                }

                return Detail::Aes256GcmProcessingResult::Succeeded;
            }


            // Streaming transform.

            /// Processes all complete AES blocks directly and buffers only one final partial block.
            Detail::Aes256GcmProcessingResult ProcessInput(
                mbedtls_gcm_context& context,
                ESPressio::Security::ByteView input,
                ESPressio::Security::MutableByteView output,
                std::size_t& producedBytes,
                std::uint8_t* tail,
                std::size_t& tailBytes
            ) const noexcept {
                producedBytes = 0U;
                tailBytes = 0U;

                if (
                    !input.IsValid() ||
                    !output.IsValid() ||
                    output.Size < input.Size
                ) {
                    return Detail::Aes256GcmProcessingResult::InvalidInput;
                }

                std::size_t offset = 0U;

                while (input.Size - offset >= BlockBytes) {
                    std::size_t blockOutput = 0U;

                    if (
                        mbedtls_gcm_update(
                            &context,
                            input.Data + offset,
                            BlockBytes,
                            output.Data + offset,
                            BlockBytes,
                            &blockOutput
                        ) != 0 ||
                        blockOutput != BlockBytes
                    ) {
                        return Detail::Aes256GcmProcessingResult::ProviderFailure;
                    }

                    offset += BlockBytes;
                }

                const auto remaining = input.Size - offset;

                if (remaining > 0U) {
                    std::size_t partialOutput = 0U;

                    if (
                        mbedtls_gcm_update(
                            &context,
                            input.Data + offset,
                            remaining,
                            tail,
                            TailScratchBytes,
                            &partialOutput
                        ) != 0 ||
                        partialOutput > TailScratchBytes
                    ) {
                        return Detail::Aes256GcmProcessingResult::ProviderFailure;
                    }

                    tailBytes = partialOutput;
                }

                producedBytes = offset;

                return Detail::Aes256GcmProcessingResult::Succeeded;
            }


            // Complete authenticated pass.

            /// Runs one complete GCM pass, optionally publishing plaintext/ciphertext into caller output.
            template<class TKeyProvider>
            Detail::Aes256GcmProcessingResult RunPass(
                int mode,
                const TKeyProvider& keys,
                const ESPressio::Security::KeySelection& selection,
                ESPressio::Security::ByteView nonce,
                ESPressio::Security::AssociatedDataView associatedData,
                ESPressio::Security::ByteView input,
                ESPressio::Security::MutableByteView output,
                std::uint8_t* calculatedTag
            ) const noexcept {
                mbedtls_gcm_context context;
                mbedtls_gcm_init(
                    &context
                );

                const auto startResult = StartOperation(
                    context,
                    mode,
                    keys,
                    selection,
                    nonce,
                    associatedData
                );

                if (startResult != Detail::Aes256GcmProcessingResult::Succeeded) {
                    mbedtls_gcm_free(
                        &context
                    );
                    return startResult;
                }

                std::uint8_t tail[TailScratchBytes]{};
                std::size_t producedBytes = 0U;
                std::size_t tailBytes = 0U;

                const auto processResult = ProcessInput(
                    context,
                    input,
                    output,
                    producedBytes,
                    tail,
                    tailBytes
                );

                if (processResult != Detail::Aes256GcmProcessingResult::Succeeded) {
                    _bytes.FillBytes(
                        tail,
                        0U,
                        sizeof(tail)
                    );
                    mbedtls_gcm_free(
                        &context
                    );
                    return processResult;
                }

                std::size_t finishBytes = 0U;

                if (
                    mbedtls_gcm_finish(
                        &context,
                        tail + tailBytes,
                        TailScratchBytes - tailBytes,
                        &finishBytes,
                        calculatedTag,
                        TagBytes
                    ) != 0 ||
                    tailBytes + finishBytes != input.Size - producedBytes
                ) {
                    _bytes.FillBytes(
                        tail,
                        0U,
                        sizeof(tail)
                    );
                    mbedtls_gcm_free(
                        &context
                    );
                    return Detail::Aes256GcmProcessingResult::ProviderFailure;
                }

                if (tailBytes + finishBytes > 0U) {
                    _bytes.CopyBytes(
                        output.Data + producedBytes,
                        tail,
                        tailBytes + finishBytes
                    );
                }

                _bytes.FillBytes(
                    tail,
                    0U,
                    sizeof(tail)
                );

                mbedtls_gcm_free(
                    &context
                );

                return Detail::Aes256GcmProcessingResult::Succeeded;
            }


            // Authentication-only pass.

            /// Calculates the GCM tag without publishing candidate plaintext to caller storage.
            template<class TKeyProvider>
            Detail::Aes256GcmProcessingResult CalculateDecryptTag(
                const TKeyProvider& keys,
                const ESPressio::Security::KeySelection& selection,
                ESPressio::Security::ByteView nonce,
                ESPressio::Security::AssociatedDataView associatedData,
                ESPressio::Security::ByteView ciphertext,
                std::uint8_t* calculatedTag
            ) const noexcept {
                mbedtls_gcm_context context;
                mbedtls_gcm_init(
                    &context
                );

                const auto startResult = StartOperation(
                    context,
                    MBEDTLS_GCM_DECRYPT,
                    keys,
                    selection,
                    nonce,
                    associatedData
                );

                if (startResult != Detail::Aes256GcmProcessingResult::Succeeded) {
                    mbedtls_gcm_free(
                        &context
                    );
                    return startResult;
                }

                std::uint8_t scratch[TailScratchBytes]{};
                std::size_t offset = 0U;

                while (offset < ciphertext.Size) {
                    const auto chunk = ciphertext.Size - offset > BlockBytes
                        ? BlockBytes
                        : ciphertext.Size - offset;
                    std::size_t outputBytes = 0U;

                    if (
                        mbedtls_gcm_update(
                            &context,
                            ciphertext.Data + offset,
                            chunk,
                            scratch,
                            sizeof(scratch),
                            &outputBytes
                        ) != 0
                    ) {
                        _bytes.FillBytes(
                            scratch,
                            0U,
                            sizeof(scratch)
                        );
                        mbedtls_gcm_free(
                            &context
                        );
                        return Detail::Aes256GcmProcessingResult::ProviderFailure;
                    }

                    offset += chunk;
                }

                std::size_t finishBytes = 0U;

                if (
                    mbedtls_gcm_finish(
                        &context,
                        scratch,
                        sizeof(scratch),
                        &finishBytes,
                        calculatedTag,
                        TagBytes
                    ) != 0
                ) {
                    _bytes.FillBytes(
                        scratch,
                        0U,
                        sizeof(scratch)
                    );
                    mbedtls_gcm_free(
                        &context
                    );
                    return Detail::Aes256GcmProcessingResult::ProviderFailure;
                }

                _bytes.FillBytes(
                    scratch,
                    0U,
                    sizeof(scratch)
                );

                mbedtls_gcm_free(
                    &context
                );

                return Detail::Aes256GcmProcessingResult::Succeeded;
            }

        public:

            // Construction.

            /// Binds the EDP-Memory byte-operation provider selected by Bootstrap.
            explicit Aes256GcmProvider(
                const TByteOperationsProvider& bytes
            ) noexcept :
                _bytes(bytes) {}


            // Authenticated encryption.

            /// Encrypts one caller-owned plaintext range and emits a 16-byte GCM tag.
            template<class TKeyProvider>
            ESPressio::Security::AuthenticatedEncryptionResult Encrypt(
                const TKeyProvider& keys,
                const ESPressio::Security::KeySelection& selection,
                ESPressio::Security::ByteView nonce,
                ESPressio::Security::AssociatedDataView associatedData,
                ESPressio::Security::ByteView plaintext,
                ESPressio::Security::MutableByteView ciphertext,
                ESPressio::Security::MutableByteView tag
            ) noexcept {
                using KeyContract = ESPressio::Security::Detail::RawKeyProviderTraits<TKeyProvider>;
                static_cast<void>(sizeof(KeyContract));

                if (
                    !selection.IsValid() ||
                    !nonce.IsValid() ||
                    nonce.Size == 0U ||
                    !associatedData.IsValid() ||
                    !plaintext.IsValid()
                ) {
                    return ESPressio::Security::AuthenticatedEncryptionResult::InvalidInput;
                }

                if (
                    !ciphertext.IsValid() ||
                    !tag.IsValid() ||
                    ciphertext.Size < plaintext.Size ||
                    tag.Size < TagBytes
                ) {
                    return ESPressio::Security::AuthenticatedEncryptionResult::OutputCapacityUnavailable;
                }

                std::uint8_t calculatedTag[TagBytes]{};
                const auto result = RunPass(
                    MBEDTLS_GCM_ENCRYPT,
                    keys,
                    selection,
                    nonce,
                    associatedData,
                    plaintext,
                    ciphertext,
                    calculatedTag
                );

                if (result == Detail::Aes256GcmProcessingResult::InvalidInput) {
                    return ESPressio::Security::AuthenticatedEncryptionResult::InvalidKey;
                }

                if (result != Detail::Aes256GcmProcessingResult::Succeeded) {
                    return ESPressio::Security::AuthenticatedEncryptionResult::ProviderFailure;
                }

                _bytes.CopyBytes(
                    tag.Data,
                    calculatedTag,
                    TagBytes
                );

                return ESPressio::Security::AuthenticatedEncryptionResult::Succeeded;
            }


            // Authenticated decryption.

            /// Authenticates ciphertext before publishing plaintext into caller storage.
            template<class TKeyProvider>
            ESPressio::Security::AuthenticatedDecryptionResult Decrypt(
                const TKeyProvider& keys,
                const ESPressio::Security::KeySelection& selection,
                ESPressio::Security::ByteView nonce,
                ESPressio::Security::AssociatedDataView associatedData,
                ESPressio::Security::ByteView ciphertext,
                ESPressio::Security::ByteView tag,
                ESPressio::Security::MutableByteView plaintext
            ) noexcept {
                using KeyContract = ESPressio::Security::Detail::RawKeyProviderTraits<TKeyProvider>;
                static_cast<void>(sizeof(KeyContract));

                if (
                    !selection.IsValid() ||
                    !nonce.IsValid() ||
                    nonce.Size == 0U ||
                    !associatedData.IsValid() ||
                    !ciphertext.IsValid() ||
                    !tag.IsValid() ||
                    tag.Size != TagBytes
                ) {
                    return ESPressio::Security::AuthenticatedDecryptionResult::InvalidInput;
                }

                if (
                    !plaintext.IsValid() ||
                    plaintext.Size < ciphertext.Size
                ) {
                    return ESPressio::Security::AuthenticatedDecryptionResult::OutputCapacityUnavailable;
                }

                std::uint8_t calculatedTag[TagBytes]{};
                const auto authenticationPass = CalculateDecryptTag(
                    keys,
                    selection,
                    nonce,
                    associatedData,
                    ciphertext,
                    calculatedTag
                );

                if (authenticationPass == Detail::Aes256GcmProcessingResult::InvalidInput) {
                    return ESPressio::Security::AuthenticatedDecryptionResult::InvalidKey;
                }

                if (authenticationPass != Detail::Aes256GcmProcessingResult::Succeeded) {
                    return ESPressio::Security::AuthenticatedDecryptionResult::ProviderFailure;
                }

                if (!IsTagEqual(
                    tag.Data,
                    calculatedTag
                )) {
                    return ESPressio::Security::AuthenticatedDecryptionResult::AuthenticationFailed;
                }

                std::uint8_t secondPassTag[TagBytes]{};
                const auto decryptionPass = RunPass(
                    MBEDTLS_GCM_DECRYPT,
                    keys,
                    selection,
                    nonce,
                    associatedData,
                    ciphertext,
                    plaintext,
                    secondPassTag
                );

                if (decryptionPass == Detail::Aes256GcmProcessingResult::InvalidInput) {
                    return ESPressio::Security::AuthenticatedDecryptionResult::InvalidKey;
                }

                return decryptionPass == Detail::Aes256GcmProcessingResult::Succeeded
                    ? ESPressio::Security::AuthenticatedDecryptionResult::Succeeded
                    : ESPressio::Security::AuthenticatedDecryptionResult::ProviderFailure;
            }

    };

} // ESPressio::Platform::ESPIDF::Security
