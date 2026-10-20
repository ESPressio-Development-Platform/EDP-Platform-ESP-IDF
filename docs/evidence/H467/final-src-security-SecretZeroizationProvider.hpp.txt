#pragma once

#include <cstddef>

#if defined(ESP_PLATFORM) && !defined(MBEDTLS_CONFIG_FILE)
#define MBEDTLS_CONFIG_FILE "mbedtls/esp_config.h"
#endif

#include <mbedtls/platform_util.h>

#include <ESPressio_Security.hpp>

namespace ESPressio::Platform::ESPIDF::Security {

    namespace Framework = ESPressio::System::CompositionFramework;

    /// ESP-IDF/Mbed TLS provider guaranteeing in-place erasure of writable secret storage.
    class SecretZeroizationProvider final : public Framework::Provider<
        ESPressio::Security::Domain,
        Framework::Offers<
            Framework::Offer<ESPressio::Security::SecretZeroization>
        >
    > {

        public:

            /// Securely erases one valid caller-owned byte range.
            ///
            /// Empty and structurally invalid views are ignored. The latter cannot be reported by
            /// the generic void-returning capability and is never supplied by a conforming caller.
            void SecureZeroize(
                ESPressio::Security::MutableByteView bytes
            ) noexcept {
                if (!bytes.IsValid() || bytes.IsEmpty()) return;

                mbedtls_platform_zeroize(
                    bytes.Data,
                    bytes.Size
                );
            }

    };


    /// Compile-time validation of the ESP-IDF SecretZeroization provider.
    using SecretZeroizationContract = ESPressio::Security::Detail::SecretZeroizationTraits<
        SecretZeroizationProvider
    >;

} // ESPressio::Platform::ESPIDF::Security
