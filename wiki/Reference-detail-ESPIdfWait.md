# src/detail/ESPIdfWait.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5/src/detail/ESPIdfWait.hpp)

## Direct includes

- `cstdint`
- `limits`
- `freertos/FreeRTOS.h`
- `freertos/semphr.h`
- `freertos/task.h`
- `ESPressio_Platform.hpp`

## Documented declarations

### `NanosecondsPerSecond`

**Classification:** PRIVATE IMPLEMENTATION

Number of nanoseconds in one second.

```cpp
inline constexpr std::uint64_t NanosecondsPerSecond = 1000000000ULL;
```

### `WaitResolutionNanoseconds`

**Classification:** PRIVATE IMPLEMENTATION

ESP-IDF FreeRTOS tick resolution rounded upward to whole nanoseconds.

```cpp
inline constexpr std::uint64_t WaitResolutionNanoseconds =
        (NanosecondsPerSecond + static_cast<std::uint64_t>(configTICK_RATE_HZ) - 1ULL) /
        static_cast<std::uint64_t>(configTICK_RATE_HZ);
```

### `ToTotalTicks`

**Classification:** PRIVATE IMPLEMENTATION

Converts a finite nanosecond duration into a rounded-up 64-bit FreeRTOS tick budget.

```cpp
inline std::uint64_t ToTotalTicks(
        ESPressio::Platform::Synchronization::WaitTimeout timeout
    ) noexcept
```

### `WaitBudget`

**Classification:** PRIVATE IMPLEMENTATION

Tracks one total ESP-IDF FreeRTOS wait budget across native semaphore acquisitions.

```cpp
class WaitBudget final
```

### `_timeout`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Original ESPressio wait request.

```cpp
ESPressio::Platform::Synchronization::WaitTimeout _timeout;
```

### `_remainingTicks`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Remaining finite wait budget expressed in native ticks.

```cpp
std::uint64_t _remainingTicks;
```

### `_lastTick`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Last observed native tick coordinate.

```cpp
TickType_t _lastTick;
```

### `ConsumeElapsed`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Removes native time elapsed since the previous budget observation.

```cpp
void ConsumeElapsed() noexcept
```

### `ConsumeWait`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Removes elapsed time after one finite native wait operation.

```cpp
void ConsumeWait(
                TickType_t before,
                TickType_t after,
                std::uint64_t requestedChunk,
                bool succeeded
            ) noexcept
```

### `WaitBudget`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Creates a wait budget from one ESPressio timeout.

```cpp
explicit WaitBudget(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept :
                _timeout(timeout),
```

### `Take`

**Classification:** PRIVATE IMPLEMENTATION · source access: `public`

Takes one ESP-IDF FreeRTOS semaphore while preserving the complete wait budget.

```cpp
bool Take(
                SemaphoreHandle_t handle
            ) noexcept
```



## Native parking extension

Production-private [ESPIdfWait.hpp](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/b009a47d77bb8c20f97c62ce702bafe058d261b2/src/detail/ESPIdfWait.hpp) additionally defines NativeReadinessWaitResult with explicit Ready=0 and TimedOut=1 uint8 outcomes. WaitBudget::WaitUntil<TPredicate> requires a noexcept predicate; NoWait inspects once, Forever polls at the existing SDK tick quantum, and finite polling consumes the SAME _remainingTicks/_lastTick accounting as preceding Take operations. It adds no clock, task, heap allocation or probe-path wait. The existing Take implementation remains unchanged.

The [execution reclamation document](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/main/docs/NATIVE-EXECUTION-RECLAMATION.MD) explains why the native owner needs a stable parked-state proof after its completion semaphore.
