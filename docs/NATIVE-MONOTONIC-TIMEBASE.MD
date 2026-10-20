# Native canonical monotonic timebase

H467 / Platform-ESP-IDF#12. The provider implements the existing EDP-Clock
`MonotonicTimebaseProvider<1000000>` capability using `esp_timer_get_time()`.
Its raw count semantics match the previous application wrappers: positive SDK
microseconds are returned unchanged; zero/negative values return zero. The
existing Clock provider owns all unit conversion. Construction has no SDK side
effects. The concrete is stateless (one byte object size), concurrent under the
reviewed SDK's normal-operation counter contract, and owns no hardware resource.

Use the narrow `<timing/EspTimerMonotonicTimebaseProvider.hpp>` header or the
native provider umbrella. Bootstrap owns the source and one canonical Clock for
the full runtime domain. Reset/deep sleep establishes another runtime domain;
never carry anchors or authoritative time across it. There is no host/RTC fallback.

## Conditional specification contract

`InspectSpecificationPrerequisites()` is an ordinary-context inspection with a
typed result. `PrerequisitesMatched` reports software/source prerequisites only.
It is neither a physical frequency measurement nor an accuracy or authority token.
A caller must preserve the reviewed SDK/source/build binding and normal-operation
conditions throughout the claimed interval. Sleep, debug halts, SDK counter
changes and out-of-band clock manipulation are excluded. A boot snapshot cannot
detect a later violation. An unqualified result must remain explicit at consumers;
this provider does not turn it into a transmit or Root eligibility boolean.

| Prerequisite | Reviewed selection |
|---|---|
| SDK | ESP-IDF 5.5.5, frozen source binding required |
| Targets | ESP32 or ESP32-S3, exactly one |
| Crystal configuration | 40 MHz |
| CPU configuration | 160 or 240 MHz; power management/tickless idle disabled |
| ESP32 counter | APB-backed TG0 LAC; configured APB 80 MHz |
| ESP32-S3 counter | XTAL-backed system timer; APB is not its source |
| Inspection | No debugger attached; source queries succeed; counter positive |

Fixed XTAL/APB frequency queries return SDK configuration-derived values. They do
not measure crystal tolerance. The manufacturer-aware specification approach
therefore remains conditional on the actual board, crystal, operating voltage,
temperature, aging and hardware satisfying the declared operating contract.
The provider deliberately retains the prior conservative application inputs:
150 ppm absolute rate error, 25,000 ns additional counter uncertainty and the
full 1,000 ns nominal read quantum. These inputs have not been independently
calibrated on the lab boards, and the inspection cannot establish their analog
compliance. No nominal frequency query is represented as that proof. Callers
must carry this qualification honestly into interval/holdover/lease policies.

The existing H466 finite experiment predates this provider. Its 22 accepted
intervals below 1 ms are observations under those declared inputs, with anchors
outside the late NVS/load window; they do not qualify concurrent accuracy,
physical rate, full ISR latency or Clock Reference authority. Root stays disabled.

## Native callback and ownership

`CurrentCount()` is the only fast path and is marked IRAM. The ordinary inspection
must not enter the IRQ capture path. Actual callback code, literals, shared data,
vendor dispatch and the exact SDK source binding still require an emitted ELF
check; the attribute alone supplies no full-ISR or latency guarantee. No extra
canonical Clock, timer, task, mutex, retained state, RF/wire field or package
version is introduced. EDP-Clock is the justified new mandatory neutral contract
dependency; it has no dependency on this concrete native repository.

## Validation

The owning host test instantiates this actual class with explicit SDK MODEL
seams on GCC/Clang. It exercises target/configuration refusal, SDK version,
debugger, failed/mismatched XTAL/APB, nonpositive counters, construction side
effects and same-counter canonical conversion. It measures no physical rate.
CTest, sanitizer, analyzer and assertion-enabled Release validation are supported.
Native builds and callback/source-binding evidence are reported separately.
The example is compile-only qualification; no device was flashed for H467.

## Primary source basis

- [ESP-IDF 5.5.5 timer API](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32/api-reference/system/esp_timer.html)
- [ESP32-S3 timer API](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32s3/api-reference/system/esp_timer.html)
- [Pinned LAC implementation](https://github.com/espressif/esp-idf/blob/v5.5.5/components/esp_timer/src/esp_timer_impl_lac.c)
- [Pinned system timer implementation](https://github.com/espressif/esp-idf/blob/v5.5.5/components/esp_timer/src/esp_timer_impl_systimer.c)
- [Pinned S3 system timer low-level source](https://github.com/espressif/esp-idf/blob/v5.5.5/components/hal/esp32s3/include/hal/systimer_ll.h)
- [Clock tree API](https://github.com/espressif/esp-idf/blob/v5.5.5/components/esp_hw_support/include/esp_clk_tree.h)
- [ESP32 hardware design guidelines](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32/schematic-checklist.html)
- [ESP32-S3 hardware design guidelines](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html)
