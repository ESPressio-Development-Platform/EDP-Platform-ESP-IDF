# Native current-context binding

ESPressio_Platform_ESP_IDF_CurrentContextBinding.hpp exposes a narrow SDK-owning realization of Platform's optional capability. It borrows one owner record through a C++20 constinit thread_local pointer. It does not consume FreeRTOS application TLS slots: the installed SDK reserves slot 0 for pthreads and has no spare slot.

Source guards require ESP32 or ESP32-S3 with native THREADPTR and C++20. A target guard is not runtime qualification. Other architectures are unsupported.

## Complete basic integration

Use one of the [three authored build surfaces](../demos/current-context-binding/README.MD), with GitHub dependencies.

1. Include the narrow header and define the owner record.

~~~cpp
#include <cstdint>
#include <ESPressio_Platform_ESP_IDF_CurrentContextBinding.hpp>

struct ContextRecord final {
    std::uint64_t identity;
    std::uint32_t slot;
};

using Binding = ESPressio::Platform::ESPIDF::Execution::CurrentContextBindingProvider<ContextRecord>;
Binding binding;
const ContextRecord applicationContext{1U, 0U};
~~~

Application System Composition selects exactly this one provider/record Type and passes it explicitly to the native lifecycle owner and capture bridge. The basic demo binds its already-running application task; actual Threading integration must bind before its eligible instrumented scopes.

2. Bind in ordinary task execution.

~~~cpp
using BindResult = ESPressio::Platform::Execution::CurrentContextBindResult;
const auto bound = binding.BindCurrent(applicationContext);
if (bound != BindResult::Bound) {
    return;
}
~~~

3. Read a typed borrow while its owner record is alive.

~~~cpp
const auto current = binding.ReadCurrent();
if (const auto* record = current.GetIfBound()) {
    (void)record->identity;
}
~~~

4. Complete captured exits, then retire exactly that record and handle any failure.

~~~cpp
using RetireResult = ESPressio::Platform::Execution::CurrentContextRetireResult;
const auto retired = binding.RetireCurrent(applicationContext);
if (retired != RetireResult::Retired) {
    // Report through ordinary application error handling.
}
~~~

No binding operation allocates, scans topology, reads a clock, performs I/O or waits for another task. Duplicate bind preserves the current pointer. Mismatched retirement preserves it. A read result does not extend the record lifetime.

## Native checks and lifetime

ISR and pre-scheduler calls return UnsupportedContext before reading compiler TLS. This defensive rejection is not an ISR-safety promise: follow project ISR policy. A suspended scheduler still has an ordinary task TLS image and is accepted.

In the installed SDK, xPortInIsrContext briefly masks/restores local interrupts to inspect nesting. xTaskGetSchedulerState has bounded cost and takes no kernel lock; its SMP variant also masks/restores local interrupts for a core-stable read. Neither waits or retries. This finite masking is part of lookup cost: lookup is not merely one pointer load.

All explicitly passed instances of this exact provider/record Type share one current-task binding across translation units, isolated between tasks. Construction does not reset it. Select one Type/domain in composition; do not use unrelated specializations as arbitrary service keys.

The SDK initializes compiler TLS inside each task stack and restores THREADPTR on switches. A pointer contributes four bytes to the 32-bit TLS image. Actual stack growth depends on alignment: the SDK aligns the complete region to 16 bytes. Budget all native tasks, including SDK tasks. Disabled firmware must not instantiate this provider and must show no related TLS symbol/image growth.

The owner supplies stable records and any nonzero/nonwrapping logical identities. Persistent Worker/Dedicated execution retains identity across dispatch and activation; a new native incarnation needs a fresh identity. OS handles/core IDs are not wire identities. The carrier does not enforce domain identities or prove abnormal termination. Never clear an active gateway flag without independent non-resumption proof.

## Qualified carrier profiles

ESP-IDF 5.5.5 and Arduino-ESP32 3.3.12, PIOArduino, Xtensa GCC 14.2.0_20260121, actual ESP32-D0WD and ESP32-S3 targets:

| Surface | Result |
|---|---|
| Actual-provider SDK MODEL, GCC/Clang Debug/Release and ASan/UBSan | PASS |
| Authored Arduino and ESP-IDF compile/link, both target families | PASS |
| Arduino IDE form on both target families | PASS |
| Real ISR rejection, interrupted-task binding preserved | PASS, four physical profiles |
| Current task isolation, cross-translation-unit reads and 20 fresh raw static-task incarnations | PASS, four profiles |
| Same native task forced from core 0 to core 1, identity preserved | PASS, four profiles |
| Suspended/resumed binding, deliberate bound-task deletion and later clean TLS | PASS, four profiles; stable parking verified before storage reuse |
| 100 actual execution-provider create/start/join/destroy cycles on alternating cores | PASS per profile, corrected native owner |
| Native linked TLS image | Four-byte pointer; zero when uninstantiated; measured empty baseline grows to 16 aligned per-task TLS bytes |
| SDK-flag assembly | THREADPTR access; bounded SDK checks and ordinary out-of-line methods; no lazy TLS/allocation/destructor helpers |

Physical receipt AI02 20261120T103923-e92b4dd6 verifies Native b009a47d77bb8c20f97c62ce702bafe058d261b2, Platform 5d1ff9c79e230d7f7e243e6d35b5e05916d9c501 and Security a7482cbc5f455d407860ad36a2b1589441674c94. These are carrier/native-owner results. They do not prove actual Threading Worker/Dedicated registration, gateway races, complete Profiling Disabled removal, application-wide stack safety or recording durability.

The native owner reclamation correction is explained [here](NATIVE-EXECUTION-RECLAMATION.MD). Original failed/incomplete fixtures and their corrections are retained in the Policy handoff; they are not silently converted to successful evidence.

## Qualification boundaries

Source implementation and actual-provider host SDK MODEL tests exist. They verify statuses, preservation, expected-record retirement, pre-scheduler/ISR branches, normal/exceptional scope exits, host TLS isolation and cross-translation-unit identity. The model uses fake SDK headers; it does not qualify native scheduling, interrupts or stack margins.

Actual Threading lifecycle integration, complete gateway races and full Profiling Disabled-image comparison remain required. Per-application stack budgets and steady-state profiling latency limits are not established by these carrier tests. Profiling is not release-ready.

[Exact provider and test symbols](../wiki/Reference-execution-CurrentContextBindingProvider.md).


## Instrumentation exclusion

The future AST overlay must exclude CurrentContextBindingProvider and its lookup/helper functions from instrumentation: a probe cannot instrument the lookup it needs to find its own context. The carrier's lower Platform result/composition helpers are likewise infrastructure. The outer native ExecutionContextProvider::NativeEntry trampoline is a lifecycle boundary; eligible Threading/application work begins inside its owner-installed binding and ends before retirement. No generator exists yet, so this is a mandatory implementation/manifest rule, not a claim that an annotation is already enforced.


## Observed lookup cost

The actual four-profile fixture measured seven warm batches of 100000 bound reads and seven unbound batches through its sole Clock::MonotonicClockProvider (existing native timebase). Median bound lookup-plus-loop/check cost was approximately 1189 ns (ESP32 ESP-IDF), 1045 ns (S3 ESP-IDF), 737 ns (ESP32 Arduino) and 628 ns (S3 Arduino). A matched empty-loop baseline is retained with every sample. These are nominal canonical-nanosecond observations on the selected configurations, including normal scheduling noise; they are neither worst-case guarantees nor complete profiling-probe costs or release acceptance limits. Receipt AI02 20261120T111837-9895536a. Production carrier/owner code remains exactly b009a47d77bb8c20f97c62ce702bafe058d261b2; maintained fixture source is 7508bb4b3b3589e7cfebc92558a2ef941852d1c3.
