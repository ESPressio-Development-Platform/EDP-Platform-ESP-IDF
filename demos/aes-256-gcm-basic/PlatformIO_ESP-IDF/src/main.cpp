#include <cstddef>
#include <cstdint>

#include <ESPressio_Memory.hpp>
#include <ESPressio_Platform_ESP_IDF.hpp>
#include <ESPressio_Platform_Portable.hpp>
#include <ESPressio_Security.hpp>

namespace Demo {

    /// Demonstrates direct AES-256-GCM provider use with a bounded borrowed key.
    int Run() noexcept {
        using ByteOperations = ESPressio::Platform::Portable::Memory::ByteOperationsProvider;
        using Crypto = ESPressio::Platform::ESPIDF::Security::Aes256GcmProvider<ByteOperations>;

        ByteOperations bytes;
        Crypto crypto(bytes);

        std::uint8_t key[32U]{};
        ESPressio::Security::BorrowedRawKeySlot slots[1U]{};
        ESPressio::Security::BorrowedRawKeyProvider keys;

        if (
            keys.Initialize(
                slots,
                1U,
                0U
            ) != ESPressio::Security::KeyProviderInitializationResult::Succeeded
        ) {
            return 1;
        }

        if (
            keys.Provision(
                ESPressio::Security::KeyId(1U),
                ESPressio::Security::KeyGeneration(1U),
                ESPressio::Security::KeyPurposeSet(
                    ESPressio::Security::KeyPurpose::DataProtection
                ),
                {
                    key,
                    sizeof(key)
                },
                ESPressio::Security::KeyProvisionDisposition::Current
            ) != ESPressio::Security::KeyProvisionResult::Succeeded
        ) {
            return 2;
        }

        const std::uint8_t nonce[12U]{
            1U, 2U, 3U, 4U, 5U, 6U,
            7U, 8U, 9U, 10U, 11U, 12U
        };
        const std::uint8_t plaintext[4U]{1U, 2U, 3U, 4U};
        std::uint8_t ciphertext[sizeof(plaintext)]{};
        std::uint8_t tag[16U]{};

        return crypto.Encrypt(
            keys,
            {
                ESPressio::Security::KeyId(1U),
                ESPressio::Security::KeyGeneration(1U),
                ESPressio::Security::KeyPurpose::DataProtection
            },
            {
                nonce,
                sizeof(nonce)
            },
            {},
            {
                plaintext,
                sizeof(plaintext)
            },
            {
                ciphertext,
                sizeof(ciphertext)
            },
            {
                tag,
                sizeof(tag)
            }
        ) == ESPressio::Security::AuthenticatedEncryptionResult::Succeeded
            ? 0
            : 3;
    }

} // Demo


extern "C" void app_main() {
    static_cast<void>(
        Demo::Run()
    );
}
