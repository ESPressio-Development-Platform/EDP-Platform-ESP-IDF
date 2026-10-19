#pragma once

#include <cstddef>
#include <cstdint>

#include <sdkconfig.h>
#include <esp_random.h>
#include <ESPressio_Platform.hpp>

#if defined(__XTENSA__) && (defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3))
#include <bootloader_random.h>
#include "../concurrency/AtomicWord32Provider.hpp"
#endif

namespace ESPressio::Platform::ESPIDF::Randomness {

    namespace Framework = ESPressio::System::CompositionFramework;

    /// Explicit ownership result; acquisition must precede every nonzero random-byte request.
    enum class InternalEntropyAcquisitionResult : std::uint8_t {
        Acquired = 0U,
        AlreadyAcquired = 1U,
        OwnedByAnotherInstance = 2U,
        UnsupportedTarget = 3U
    };

    enum class InternalEntropyReleaseResult : std::uint8_t {
        Released = 0U,
        NotOwned = 1U
    };

    /// Cryptographically suitable bytes ONLY while this instance owns the SDK main entropy source.
    /// Bootstrap explicitly reserves ADC/internal RF and ESP32 I2S before acquisition, and joins
    /// consumers/releases the source before using those peripherals. Acquisition/release are
    /// externally serialized ordinary-context operations; never call them from an ISR.
    /// No peripheral is enabled implicitly by construction or FillRandomBytes.
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
    private:
        /// True only after this instance acquired the global owner and actually enabled SDK entropy.
        bool _ownsInternalEntropy{false};

#if defined(__XTENSA__) && (defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3))
        /// Existing qualified native four-byte atomic, inline once across all translation units.
        /// Complete storage must remain internal DRAM; the owning atomic enforces that at construction.
        inline static Concurrency::AtomicWord32Provider::Word _sourceOwner{};
#endif

    public:
        /// Construction has no ADC/I2S/RF side effect and leaves nonzero requests unavailable.
        RandomByteSourceProvider() noexcept = default;
        RandomByteSourceProvider(const RandomByteSourceProvider&) = delete;
        RandomByteSourceProvider& operator=(const RandomByteSourceProvider&) = delete;
        RandomByteSourceProvider(RandomByteSourceProvider&&) = delete;
        RandomByteSourceProvider& operator=(RandomByteSourceProvider&&) = delete;

        /// Releases only an actually owned source. Consumers must already have stopped.
        ~RandomByteSourceProvider() noexcept {
            (void)ReleaseInternalEntropySource();
        }

        /// Explicit Bootstrap resource acquisition for the audited ESP32/S3 no-internal-RF mode.
        /// Caller MUST ensure no ADC/internal RF use, nor ESP32 I2S, for the complete owned lifetime.
        /// The SDK changes those peripherals; this is a reservation, not a passive suitability flag.
        InternalEntropyAcquisitionResult AcquireInternalEntropySource() noexcept {
#if defined(__XTENSA__) && (defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3))
            if (_ownsInternalEntropy) return InternalEntropyAcquisitionResult::AlreadyAcquired;
            std::uint32_t expected = 0U;
            if (!_sourceOwner.CompareExchangeAcqRel(expected,1U)) {
                return InternalEntropyAcquisitionResult::OwnedByAnotherInstance;
            }
            bootloader_random_enable();
            _ownsInternalEntropy = true;
            return InternalEntropyAcquisitionResult::Acquired;
#else
            return InternalEntropyAcquisitionResult::UnsupportedTarget;
#endif
        }

        /// Restores SDK entropy peripherals before publishing the global ownership release.
        InternalEntropyReleaseResult ReleaseInternalEntropySource() noexcept {
#if defined(__XTENSA__) && (defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3))
            if (_ownsInternalEntropy) {
                bootloader_random_disable();
                _ownsInternalEntropy = false;
                _sourceOwner.StoreRelease(0U);
                return InternalEntropyReleaseResult::Released;
            }
#endif
            return InternalEntropyReleaseResult::NotOwned;
        }

        /// Reports this instance's actual SDK resource ownership, not measured entropy quality.
        bool OwnsInternalEntropySource() const noexcept { return _ownsInternalEntropy; }

        /// Fills caller storage only under an explicitly held, lifetime-valid main entropy source.
        /// Zero bytes need no source; invalid destination and unavailable/unsupported source refuse.
        /// No pseudo-random fallback, RF-state guess, hidden source acquisition or new RNG algorithm.
        ESPressio::Platform::Randomness::RandomByteGenerationResult FillRandomBytes(
            std::uint8_t* destination,
            std::size_t byteCount
        ) noexcept {
            using Result = ESPressio::Platform::Randomness::RandomByteGenerationResult;
            if (byteCount == 0U) return Result::Succeeded;
            if (destination == nullptr) return Result::InvalidDestination;
#if defined(__XTENSA__) && (defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3))
            if (_ownsInternalEntropy) {
                esp_fill_random(destination,byteCount);
                return Result::Succeeded;
            }
#endif
            return Result::ProviderFailure;
        }
    };

    /// Validates the existing generic capability, features and typed nonthrowing operation.
    using RandomByteSourceContract = ESPressio::Platform::Randomness::Detail::RandomByteSourceProviderTraits<RandomByteSourceProvider>;

} // namespace ESPressio::Platform::ESPIDF::Randomness
