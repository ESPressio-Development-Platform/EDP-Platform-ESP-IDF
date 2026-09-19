# ESP-IDF ExecutionContext Provider

## Purpose and scope

`EDP-Platform-ESP-IDF` supplies the ESP-IDF-specific concrete provider for the `EDP-Platform` `ExecutionContext` capability.

The provider exists because ESP-IDF exposes `xTaskCreateStaticPinnedToCore`, which satisfies the Platform execution contract while also supporting processor affinity. Higher Task, Thread, pooling, loop, cancellation, and scheduler semantics remain outside this repository.

## Availability

The provider is compiled only when the ESP-IDF FreeRTOS configuration provides every native facility required by the current contract:

```text
configSUPPORT_STATIC_ALLOCATION == 1
INCLUDE_vTaskDelete == 1
INCLUDE_vTaskSuspend == 1
INCLUDE_xTaskGetCurrentTaskHandle == 1
```

Stack telemetry is advertised separately. `SupportsStackTelemetry` is true only when `INCLUDE_uxTaskGetStackHighWaterMark == 1`.

## Composition contract

The provider offers `ESPressio::Platform::Execution::ExecutionContext` and advertises:

```text
CallerSuppliedStorage              = true
SupportsPriority                  = true
SupportsProcessorAffinity         = true
SupportsStackTelemetry            = configuration-dependent
ControlStorageBytes               = sizeof(StaticTask_t)
ControlStorageAlignment           = alignof(StaticTask_t)
StackStorageAlignment             = alignof(StackType_t)
StackAllocationGranularityBytes   = sizeof(StackType_t)
JoinWaitResolutionNanoseconds     = ceil(1 second / configTICK_RATE_HZ)
```

Application Composition selects this provider when ESP-IDF-specific affinity support is required. The provider object remains owned by application Bootstrap or another ordinary C++ runtime owner; Composition does not construct or retain it.

## Static storage and memory ownership

`Initialize()` requires caller-supplied task-control storage and caller-supplied stack storage.

The supplied control region must be large and aligned enough for `StaticTask_t`. The supplied stack region must be aligned for `StackType_t`, contain at least one stack element, fit the ESP-IDF byte-sized stack-depth parameter, and be an exact multiple of the native stack element size.

The provider also owns two `StaticSemaphore_t` objects internally for:

```text
start gate
completion notification
```

Those semaphores are created through the FreeRTOS static-semaphore API. The provider does not allocate the native task control block or task stack from the FreeRTOS heap.

## Initialization

`Initialize()` binds:

```text
caller storage
priority
processor affinity
optional name
entry function
entry parameter
```

The portable `ExecutionPriority` values `Low`, `Normal`, `High`, and `Critical` are mapped monotonically across the configured native ESP-IDF FreeRTOS priority range. `Critical` maps to the highest available native priority; `Low` avoids the idle priority where the native range permits. Small native priority ranges may collapse adjacent logical levels. The entry function must be non-null.

`ProcessorAffinity::Any()` maps to `tskNO_AFFINITY`. A specific processor index is accepted only when ESP-IDF reports it as a valid core through `taskVALID_CORE_ID`.

Important result states include:

```text
Succeeded
AlreadyInitialized
UnsupportedAffinity
InvalidConfiguration
InvalidStorage
ProviderFailure
```

A failed initialization does not publish a successfully initialized execution context.

## Lifecycle

The lifecycle is:

```text
Initialize
Start
Join
Destroy
```

The native task is created during `Initialize()` but begins behind a private start gate. User code does not execute until `Start()` releases that gate.

When the entry function returns, the native task signals completion and then suspends itself. It does not delete itself.

`Join()` waits for completion and records that ownership has been joined. A context cannot join itself. Repeated successful joins are idempotent.

`Destroy()` succeeds only for:

```text
an initialized context that was never started
or
a started context whose completed execution has been joined
```

It deliberately refuses to forcibly terminate a running or unjoined execution scope. On successful destruction, `vTaskDelete()` retires the native task and the provider returns to its uninitialized state.

## Wait semantics

`Join()` consumes the Platform `WaitTimeout` abstraction.

Finite nanosecond waits are converted to FreeRTOS ticks by rounding upward so unit conversion cannot cause an earlier timeout than requested. Long finite waits are consumed as a total wait budget rather than resetting the requested duration on each internal wait operation.

`WaitForever` maps to an indefinite FreeRTOS semaphore wait because provider availability already requires `INCLUDE_vTaskSuspend == 1`.

## Stack telemetry

When stack high-water telemetry is enabled, `GetStackTelemetry()` reports the ESP-IDF high-water result as minimum free stack bytes.

When the configuration does not expose the native telemetry API, the provider advertises `SupportsStackTelemetry = false` and returns an unavailable telemetry reading rather than inventing a value.

## Current-context inspection and yielding

`IsCurrentContext()` compares the native task handle with `xTaskGetCurrentTaskHandle()`.

`Yield()` maps directly to `taskYIELD()` and provides only the Platform-native yield primitive. It does not introduce ESPressio Threading semantics.

## Supported and unsupported behavior

Supported:

```text
caller-supplied static task storage
priority
optional processor affinity
join with ESPressio wait policy
conditional stack telemetry
explicit native yield
```

Not provided by this provider:

```text
forced cancellation
automatic task deletion after entry return
dynamic task-stack allocation through the Platform API
Thread or Task semantic ownership
pooling
scheduling policy above the native ESP-IDF priority/affinity fields
```

These higher semantics belong to higher ESPressio domains such as `EDP-Threading`.
