#include <array>
#include <cassert>
#include <cstdint>
#include <type_traits>

#include <randomness/RandomByteSourceProvider.hpp>

namespace Test {

    /// Host has no qualified native entropy backend. Verify refusal without invoking the SDK stub.
    int Run() noexcept {
        using namespace ESPressio::Platform::ESPIDF::Randomness;
        using Provider = RandomByteSourceProvider;
        using Result = ESPressio::Platform::Randomness::RandomByteGenerationResult;
        using Contract = ESPressio::Platform::Randomness::Detail::RandomByteSourceProviderTraits<Provider>;
        using Properties = typename Contract::Properties;
        constexpr ESPressio::System::FlagSet<
            ESPressio::Platform::Randomness::RandomByteSourceFeature,std::uint8_t> features(
            Properties::template Value<ESPressio::Platform::Randomness::RandomByteSourceFeatures>);
        static_assert(features.HasAll(ESPressio::Platform::Randomness::RandomByteSourceFeature::CryptographicallySuitable));
        static_assert(!std::is_copy_constructible_v<Provider> && !std::is_move_constructible_v<Provider>);
        static_assert(!std::is_copy_assignable_v<Provider> && !std::is_move_assignable_v<Provider>);

        Provider first,second;
        std::array<std::uint8_t,8U> bytes{};
        bytes.fill(0x3CU);
        const auto original=bytes;
        assert(!first.OwnsInternalEntropySource());
        assert(first.FillRandomBytes(bytes.data(),bytes.size())==Result::ProviderFailure);
        assert(bytes==original && EspRandomStubCalls==0U);
        assert(first.AcquireInternalEntropySource()==InternalEntropyAcquisitionResult::UnsupportedTarget);
        assert(second.AcquireInternalEntropySource()==InternalEntropyAcquisitionResult::UnsupportedTarget);
        assert(!first.OwnsInternalEntropySource() && !second.OwnsInternalEntropySource());
        assert(first.ReleaseInternalEntropySource()==InternalEntropyReleaseResult::NotOwned);
        assert(second.ReleaseInternalEntropySource()==InternalEntropyReleaseResult::NotOwned);
        assert(first.FillRandomBytes(nullptr,1U)==Result::InvalidDestination);
        assert(first.FillRandomBytes(nullptr,0U)==Result::Succeeded);
        assert(first.FillRandomBytes(bytes.data(),0U)==Result::Succeeded);
        assert(bytes==original && EspRandomStubCalls==0U);
        return 0;
    }
}

int main() { return Test::Run(); }
