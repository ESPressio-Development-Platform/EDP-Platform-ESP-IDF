# ESP-IDF SpinLock Provider

## Purpose and scope

`EDP-Platform-ESP-IDF` supplies the ESP-IDF-specific concrete provider for the `EDP-Platform` `SpinLock` capability.

The provider wraps ESP-IDF's SMP-aware `portMUX_TYPE` critical-section primitive. It is intended only for extremely short non-blocking critical regions where a spin/critical-section primitive is specifically required.

## Composition contract

The provider offers:

```text
ESPressio::Platform::Synchronization::SpinLock
```

and advertises:

```text
SpinLockSupportsInterruptContext = true
```

The provider is selected through application Composition. Runtime ownership remains ordinary C++ ownership outside the Composition Framework.

## Native representation

The provider owns one inline:

```cpp
portMUX_TYPE
```

initialized with `portMUX_INITIALIZER_UNLOCKED`.

There is no heap allocation, registry, worker task, or external synchronization object owned by the provider.

## Ordinary-context behavior

`Acquire()` enters the ESP-IDF critical section through `portENTER_CRITICAL` and spins as required by the native implementation until the mux is acquired.

`Release()` leaves the critical section through `portEXIT_CRITICAL` and reports `SpinLockReleaseResult::Released`.

The provider does not add a separate runtime ownership tracker. Correct acquire/release pairing remains a caller contract enforced by the underlying ESP-IDF primitive.

## Interrupt-context behavior

`AcquireFromInterrupt()` and `ReleaseFromInterrupt()` use the corresponding ESP-IDF ISR critical-section APIs:

```text
portENTER_CRITICAL_ISR
portEXIT_CRITICAL_ISR
```

This is why the provider may truthfully advertise interrupt-context support.

## Interrupt masking and scheduling implications

Holding the lock enters an ESP-IDF critical section and masks interrupts to the level defined by the ESP-IDF port while the critical section remains active.

On multi-core targets, acquisition may also spin while another core owns the mux.

Therefore protected code must remain:

```text
short
non-blocking
non-yielding
free of waits that depend on another execution context progressing
```

Using this provider around long work, I/O, semaphore waits, task joins, or any operation that may block can damage latency and may create liveness failures.

## Boundary with other synchronization providers

`SpinLock` is not a substitute for the Platform `Mutex`, `RecursiveMutex`, `ReadWriteMutex`, `CountingSemaphore`, or `Signal` capabilities.

Those abstractions carry different waiting and ownership semantics. In an ESP-IDF application, portable FreeRTOS-backed implementations of those capabilities remain owned by `EDP-Platform-FreeRTOS`, because provider ownership follows the API that implements the abstraction.
