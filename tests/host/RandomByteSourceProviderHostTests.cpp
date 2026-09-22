#include <cassert>
#include <cstdint>

#include <randomness/RandomByteSourceProvider.hpp>

namespace Test {

    /// Exercises the ESP-IDF RandomByteSource provider against the host ESP-IDF stub.
    int Run() noexcept {
        using Provider = ESPressio::Platform::ESPIDF::Randomness::RandomByteSourceProvider;
        using Contract = ESPressio::Platform::Randomness::Detail::RandomByteSourceProviderTraits<Provider>;
        using Properties = typename Contract::Properties;

        constexpr ESPressio::System::FlagSet<
            ESPressio::Platform::Randomness::RandomByteSourceFeature,
            std::uint8_t
        > features(
            Properties::template Value<ESPressio::Platform::Randomness::RandomByteSourceFeatures>
        );

        static_assert(
            features.HasAll(
                ESPressio::Platform::Randomness::RandomByteSourceFeature::CryptographicallySuitable
            ),
            "ESP-IDF random source must advertise cryptographic suitability"
        );

        Provider provider;
        std::uint8_t bytes[8U]{};

        assert(
            provider.FillRandomBytes(
                bytes,
                sizeof(bytes)
            ) == ESPressio::Platform::Randomness::RandomByteGenerationResult::Succeeded
        );

        assert(bytes[0U] == 0xA5U);
        assert(bytes[1U] == 0xA4U);

        assert(
            provider.FillRandomBytes(
                nullptr,
                1U
            ) == ESPressio::Platform::Randomness::RandomByteGenerationResult::InvalidDestination
        );

        assert(
            provider.FillRandomBytes(
                nullptr,
                0U
            ) == ESPressio::Platform::Randomness::RandomByteGenerationResult::Succeeded
        );

        return 0;
    }

} // Test


/// Runs ESP-IDF random provider host tests.
int main() {
    return Test::Run();
}
