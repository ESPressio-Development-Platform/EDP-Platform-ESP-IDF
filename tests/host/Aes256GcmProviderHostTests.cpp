#include <cassert>
#include <cstddef>
#include <cstdint>

#include <security/Aes256GcmProvider.hpp>

namespace Test {

    namespace Framework = ESPressio::System::CompositionFramework;

    /// Host ByteOperations provider for the ESP-IDF AES-GCM provider test.
    class TestByteOperations final : public Framework::Provider<
        ESPressio::Memory::Domain,
        Framework::Provides<
            Framework::Offer<ESPressio::Memory::ByteOperations>
        >
    > {

        public:

            void CopyBytes(void* destination, const void* source, std::size_t count) const noexcept {
                auto* out = static_cast<std::uint8_t*>(destination);
                const auto* in = static_cast<const std::uint8_t*>(source);

                for (std::size_t index = 0U; index < count; ++index) out[index] = in[index];
            }

            void MoveBytes(void* destination, const void* source, std::size_t count) const noexcept {
                auto* out = static_cast<std::uint8_t*>(destination);
                const auto* in = static_cast<const std::uint8_t*>(source);

                if (out < in) {
                    for (std::size_t index = 0U; index < count; ++index) out[index] = in[index];
                } else if (out > in) {
                    for (std::size_t index = count; index > 0U; --index) out[index - 1U] = in[index - 1U];
                }
            }

            void FillBytes(void* destination, std::uint8_t value, std::size_t count) const noexcept {
                auto* out = static_cast<std::uint8_t*>(destination);
                for (std::size_t index = 0U; index < count; ++index) out[index] = value;
            }

            ESPressio::Memory::ByteComparison CompareBytes(
                const void* left,
                const void* right,
                std::size_t count
            ) const noexcept {
                const auto* a = static_cast<const std::uint8_t*>(left);
                const auto* b = static_cast<const std::uint8_t*>(right);

                for (std::size_t index = 0U; index < count; ++index) {
                    if (a[index] < b[index]) return ESPressio::Memory::ByteComparison::Less;
                    if (a[index] > b[index]) return ESPressio::Memory::ByteComparison::Greater;
                }

                return ESPressio::Memory::ByteComparison::Equal;
            }

    };


    /// Exercises provider metadata plus segmented-AAD encrypt/decrypt and authentication rejection.
    int Run() noexcept {
        TestByteOperations bytes;
        using Provider = ESPressio::Platform::ESPIDF::Security::Aes256GcmProvider<TestByteOperations>;
        using Contract = ESPressio::Security::Detail::AuthenticatedCryptoProviderTraits<Provider>;

        static_assert(Contract::Algorithm == ESPressio::Security::AuthenticatedAlgorithm::Aes256Gcm);
        static_assert(Contract::KeyBytes == 32U);
        static_assert(Contract::DefaultNonceBytes == 12U);
        static_assert(Contract::TagBytes == 16U);

        Provider crypto(bytes);

        std::uint8_t key[32U]{};
        for (std::size_t index = 0U; index < sizeof(key); ++index) {
            key[index] = static_cast<std::uint8_t>(index + 1U);
        }

        ESPressio::Security::BorrowedRawKeySlot slots[1U]{};
        ESPressio::Security::BorrowedRawKeyProvider keys;

        assert(
            keys.Initialize(
                slots,
                1U,
                0U
            ) == ESPressio::Security::KeyProviderInitializationResult::Succeeded
        );

        assert(
            keys.Provision(
                ESPressio::Security::KeyId(1U),
                ESPressio::Security::KeyGeneration(1U),
                ESPressio::Security::KeyPurposeSet(
                    ESPressio::Security::KeyPurpose::DataProtection,
                    ESPressio::Security::KeyPurpose::TransportProtection
                ),
                {
                    key,
                    sizeof(key)
                },
                ESPressio::Security::KeyProvisionDisposition::Current
            ) == ESPressio::Security::KeyProvisionResult::Succeeded
        );

        const std::uint8_t nonce[12U]{
            1U, 2U, 3U, 4U, 5U, 6U,
            7U, 8U, 9U, 10U, 11U, 12U
        };
        const std::uint8_t aadOne[2U]{0xAAU, 0xBBU};
        const std::uint8_t aadTwo[3U]{0xCCU, 0xDDU, 0xEEU};
        const ESPressio::Security::ByteView aad[2U]{
            {aadOne, sizeof(aadOne)},
            {aadTwo, sizeof(aadTwo)}
        };
        const std::uint8_t plaintext[19U]{
            1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U,
            11U, 12U, 13U, 14U, 15U, 16U, 17U, 18U, 19U
        };
        std::uint8_t ciphertext[sizeof(plaintext)]{};
        std::uint8_t tag[16U]{};

        const ESPressio::Security::KeySelection selection{
            ESPressio::Security::KeyId(1U),
            ESPressio::Security::KeyGeneration(1U),
            ESPressio::Security::KeyPurpose::DataProtection
        };

        assert(
            crypto.Encrypt(
                keys,
                selection,
                {nonce, sizeof(nonce)},
                {aad, 2U},
                {plaintext, sizeof(plaintext)},
                {ciphertext, sizeof(ciphertext)},
                {tag, sizeof(tag)}
            ) == ESPressio::Security::AuthenticatedEncryptionResult::Succeeded
        );

        std::uint8_t recovered[sizeof(plaintext)]{};

        assert(
            crypto.Decrypt(
                keys,
                selection,
                {nonce, sizeof(nonce)},
                {aad, 2U},
                {ciphertext, sizeof(ciphertext)},
                {tag, sizeof(tag)},
                {recovered, sizeof(recovered)}
            ) == ESPressio::Security::AuthenticatedDecryptionResult::Succeeded
        );

        for (std::size_t index = 0U; index < sizeof(plaintext); ++index) {
            assert(recovered[index] == plaintext[index]);
        }

        tag[0U] ^= 0x01U;
        bytes.FillBytes(
            recovered,
            0x5AU,
            sizeof(recovered)
        );

        assert(
            crypto.Decrypt(
                keys,
                selection,
                {nonce, sizeof(nonce)},
                {aad, 2U},
                {ciphertext, sizeof(ciphertext)},
                {tag, sizeof(tag)},
                {recovered, sizeof(recovered)}
            ) == ESPressio::Security::AuthenticatedDecryptionResult::AuthenticationFailed
        );

        for (const auto value : recovered) {
            assert(value == 0x5AU);
        }

        return 0;
    }

} // Test


int main() {
    return Test::Run();
}
