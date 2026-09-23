#pragma once

#include <cstddef>
#include <cstdint>

#include <esp_random.h>

#include <ESPressio_Platform.hpp>

namespace ESPressio::Platform::ESPIDF::Randomness {

    namespace Framework = ESPressio::System::CompositionFramework;

    /// ESP-IDF hardware-random provider suitable for cryptographic random-byte requests.
    class RandomByteSourceProvider final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Offers<
            Framework::Offer<
                ESPressio::Platform::Randomness::RandomByteSource,
                Framework::FlagPropertyValue<
                    ESPressio::Platform::Randomness::RandomByteSourceFeatures,
                    ESPressio::Platform::Randomness::RandomByteSourceFeature::CryptographicallySuitable
                >
            >
        >
    > {

        public:

            // Random-byte generation.

            /// Fills caller-owned storage using ESP-IDF's system random source.
            ///
            /// A zero-byte request succeeds without accessing destination.
            ///
            /// @param destination Caller-owned writable byte range.
            /// @param byteCount Number of random bytes requested.
            ESPressio::Platform::Randomness::RandomByteGenerationResult FillRandomBytes(
                std::uint8_t* destination,
                std::size_t byteCount
            ) noexcept {
                if (byteCount == 0U) {
                    return ESPressio::Platform::Randomness::RandomByteGenerationResult::Succeeded;
                }

                if (destination == nullptr) {
                    return ESPressio::Platform::Randomness::RandomByteGenerationResult::InvalidDestination;
                }

                esp_fill_random(
                    destination,
                    byteCount
                );

                return ESPressio::Platform::Randomness::RandomByteGenerationResult::Succeeded;
            }

    };


    /// Compile-time validation of the ESP-IDF RandomByteSource provider.
    using RandomByteSourceContract = ESPressio::Platform::Randomness::Detail::RandomByteSourceProviderTraits<RandomByteSourceProvider>;

} // ESPressio::Platform::ESPIDF::Randomness
