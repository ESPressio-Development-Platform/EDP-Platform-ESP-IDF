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

## Qualification state

Source implementation and actual-provider host SDK MODEL tests exist. They verify statuses, preservation, expected-record retirement, pre-scheduler/ISR branches, normal/exceptional scope exits, host TLS isolation and cross-translation-unit identity. The model uses fake SDK headers; it does not qualify native scheduling, interrupts or stack margins.

SDK-linked Arduino/ESP-IDF, physical WROOM/S3, stack/image budgets, Disabled-image comparison, actual owner integration and gateway races remain required until separately recorded as PASS. Profiling is not release-ready.

[Exact provider and test symbols](../wiki/Reference-execution-CurrentContextBindingProvider.md).
