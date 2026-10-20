#pragma once

#include <cstdint>
#include <type_traits>
#include <clock/MonotonicTimebaseProvider.hpp>

#if defined(ESP_PLATFORM)
#include <sdkconfig.h>
#include <esp_attr.h>
#include <esp_clk_tree.h>
#include <esp_cpu.h>
#include <esp_idf_version.h>
#include <esp_timer.h>

namespace ESPressio::Platform::ESPIDF::Timing {

    /// Configuration/source checks only: no measured oscillator or accuracy certificate.
    enum class TimebaseSpecificationInspectionResult : std::uint8_t {
        PrerequisitesMatched = 0U,
        UnsupportedTarget = 1U,
        UnsupportedSdkVersion = 2U,
        UnsupportedBuildConfiguration = 3U,
        DebuggerAttached = 4U,
        ClockFrequencyQueryFailed = 5U,
        CrystalFrequencyMismatch = 6U,
        ApbFrequencyMismatch = 7U,
        CounterUnavailable = 8U
    };

    /// Identifies the pinned SDK counter path, not a newly allocated hardware resource.
    enum class EspTimerCounterSource : std::uint8_t {
        Unsupported = 0U,
        Esp32ApbLac = 1U,
        Esp32S3XtalSystemTimer = 2U
    };

    /// Existing SDK microsecond counter offered through the existing Clock timebase capability.
    /// Bootstrap owns one canonical MonotonicClock using this stateless provider. No SDK timer
    /// is created, started, set or stopped. SDK initialization must already have completed.
    /// The source is coherent/concurrent under the supported normal-operation SDK contract;
    /// reset/deep-sleep starts a different runtime domain. No host or RTC fallback exists.
    class EspTimerMonotonicTimebaseProvider final :
        public ESPressio::Clock::MonotonicTimebaseProvider<1'000'000U> {
    public:
        /// Nominal read quantum; this is not independently measured physical accuracy.
        static constexpr std::uint32_t NominalResolutionNanoseconds = 1'000U;
        /// Conservative application inputs retained from the reviewed capture policy.
        /// Physical hardware must separately satisfy its declared operating contract.
        static constexpr std::uint32_t SuggestedAbsoluteRateErrorPpm = 150U;
        static constexpr std::uint32_t SuggestedCounterAdditionalUncertaintyNanoseconds = 25'000U;

        /// Mirrors the existing application wrapper. Inspection never changes this counter.
        /// This fast path is usable by the canonical IRAM capture callback after placement proof.
        IRAM_ATTR std::uint64_t CurrentCount() const noexcept {
            const auto now = esp_timer_get_time();
            return now > 0 ? static_cast<std::uint64_t>(now) : 0U;
        }

        /// Compile-time source identity for the reviewed target selection.
        static constexpr EspTimerCounterSource CounterSource() noexcept {
#if defined(CONFIG_IDF_TARGET_ESP32) && CONFIG_IDF_TARGET_ESP32 && \
    !(defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3)
            return EspTimerCounterSource::Esp32ApbLac;
#elif defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3 && \
    !(defined(CONFIG_IDF_TARGET_ESP32) && CONFIG_IDF_TARGET_ESP32)
            return EspTimerCounterSource::Esp32S3XtalSystemTimer;
#else
            return EspTimerCounterSource::Unsupported;
#endif
        }

        /// Ordinary-context snapshot of software/configuration prerequisites ONLY.
        /// Matched does not prove crystal tolerance, analog hardware compliance, uninterrupted
        /// future operation, independent calibration, IRQ latency, ClockEra or lease authority.
        /// The conditional specification contract also excludes sleep, debug halts and external
        /// counter changes throughout use; callers must preserve the reviewed firmware binding.
        TimebaseSpecificationInspectionResult InspectSpecificationPrerequisites() const noexcept {
            using Result = TimebaseSpecificationInspectionResult;
            if constexpr (CounterSource() == EspTimerCounterSource::Unsupported) {
                return Result::UnsupportedTarget;
            }
#if ESP_IDF_VERSION != ESP_IDF_VERSION_VAL(5, 5, 5)
            return Result::UnsupportedSdkVersion;
#elif !defined(CONFIG_XTAL_FREQ) || CONFIG_XTAL_FREQ != 40 || \
      !defined(CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ) || \
      (CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ != 160 && CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ != 240) || \
      (defined(CONFIG_PM_ENABLE) && CONFIG_PM_ENABLE) || \
      (defined(CONFIG_FREERTOS_USE_TICKLESS_IDLE) && CONFIG_FREERTOS_USE_TICKLESS_IDLE)
            return Result::UnsupportedBuildConfiguration;
#else
#if defined(CONFIG_IDF_TARGET_ESP32) && CONFIG_IDF_TARGET_ESP32
#if !defined(CONFIG_ESP_TIMER_IMPL_TG0_LAC) || !CONFIG_ESP_TIMER_IMPL_TG0_LAC
            return Result::UnsupportedBuildConfiguration;
#endif
#elif defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3
#if !defined(CONFIG_ESP_TIMER_IMPL_SYSTIMER) || !CONFIG_ESP_TIMER_IMPL_SYSTIMER
            return Result::UnsupportedBuildConfiguration;
#endif
#endif
            if (esp_cpu_dbgr_is_attached()) { return Result::DebuggerAttached; }
            std::uint32_t frequency = 0U;
            // For fixed XTAL/APB sources the SDK returns configuration-derived frequency;
            // the precision selector does not perform physical frequency calibration.
            if (esp_clk_tree_src_get_freq_hz(SOC_MOD_CLK_XTAL,
                    ESP_CLK_TREE_SRC_FREQ_PRECISION_CACHED, &frequency) != ESP_OK) {
                return Result::ClockFrequencyQueryFailed;
            }
            if (frequency != 40'000'000U) { return Result::CrystalFrequencyMismatch; }
            if constexpr (CounterSource() == EspTimerCounterSource::Esp32ApbLac) {
                if (esp_clk_tree_src_get_freq_hz(SOC_MOD_CLK_APB,
                        ESP_CLK_TREE_SRC_FREQ_PRECISION_CACHED, &frequency) != ESP_OK) {
                    return Result::ClockFrequencyQueryFailed;
                }
                if (frequency != 80'000'000U) { return Result::ApbFrequencyMismatch; }
            }
            return CurrentCount() != 0U ? Result::PrerequisitesMatched : Result::CounterUnavailable;
#endif
        }
    };

    static_assert(std::is_empty_v<EspTimerMonotonicTimebaseProvider>);
    static_assert(sizeof(EspTimerMonotonicTimebaseProvider) == 1U);

} // namespace ESPressio::Platform::ESPIDF::Timing
#endif
