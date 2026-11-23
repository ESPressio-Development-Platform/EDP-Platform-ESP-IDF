# Native ESP timer monotonic timebase reference

Source: src/timing/EspTimerMonotonicTimebaseProvider.hpp. PUBLIC OPTIONAL provider in ESPressio::Platform::ESPIDF::Timing, available only under ESP_PLATFORM. It derives from the existing Clock::MonotonicTimebaseProvider<1000000U>; one canonical application Clock composes it. It allocates no timer, clock service or task and retains no per-instance state.

TimebaseSpecificationInspectionResult is uint8_t: PrerequisitesMatched=0, UnsupportedTarget=1, UnsupportedSdkVersion=2, UnsupportedBuildConfiguration=3, DebuggerAttached=4, ClockFrequencyQueryFailed=5, CrystalFrequencyMismatch=6, ApbFrequencyMismatch=7, CounterUnavailable=8. These are software/configuration inspection facts, not physical accuracy certification.

EspTimerCounterSource is uint8_t: Unsupported=0, Esp32ApbLac=1, Esp32S3XtalSystemTimer=2. It identifies the existing SDK source.

EspTimerMonotonicTimebaseProvider constants are NominalResolutionNanoseconds=1000, SuggestedAbsoluteRateErrorPpm=150 and SuggestedCounterAdditionalUncertaintyNanoseconds=25000, all uint32_t. They are reviewed nominal/conservative inputs, not independently measured hardware guarantees.

CurrentCount() const noexcept -> uint64_t is IRAM_ATTR and reads esp_timer_get_time, converting a positive count unchanged and returning zero otherwise. It does not inspect or reconfigure the source.

CounterSource() static constexpr noexcept -> EspTimerCounterSource selects the mutually exclusive ESP32 or ESP32-S3 configuration; other/ambiguous combinations return Unsupported.

InspectSpecificationPrerequisites() const noexcept -> TimebaseSpecificationInspectionResult checks target, exact ESP-IDF5.5.5, reviewed crystal/CPU/power/tickless configuration, expected counter backend, debugger state, frequency-query success and exact configuration frequencies, then nonzero availability. It never calibrates, mutates the counter, proves future continuity or supplies ClockEra/election authority. In particular a later SDK does not satisfy its exact specification check merely because CurrentCount compiles.

The two static assertions require an empty provider and sizeof=1. No constructor/custom copy/move members or private fields/helpers are declared. Reset/deep sleep changes runtime domain. See [normative specification and physical limits](../docs/NATIVE-MONOTONIC-TIMEBASE.MD).
