# src/synchronization/SpinLockProvider.hpp

**Primary classification:** PUBLIC PROVIDER / EXTENSION API

**Source baseline:** `3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5/src/synchronization/SpinLockProvider.hpp)

## Direct includes

- `freertos/FreeRTOS.h`
- `freertos/portmacro.h`
- `ESPressio_Platform.hpp`

## Documented declarations

### `SpinLockProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

ESP-IDF portMUX-backed spin-lock provider.

Acquisition enters an ESP-IDF critical section and therefore masks interrupts to the
configured critical level while the lock is held. Callers must keep the protected scope short.

```cpp
class SpinLockProvider final : public Framework::Provider<
        ESPressio::Platform::Domain,
```

### `_lock`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

ESP-IDF SMP-aware critical-section spin lock.

```cpp
portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;
```

### `SpinLockProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Creates an unlocked ESP-IDF spin lock.

```cpp
SpinLockProvider() noexcept = default;
```

### `SpinLockProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents copying native spin-lock state.

```cpp
SpinLockProvider(const SpinLockProvider&) = delete;
```

### `operator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents copy assignment of native spin-lock state.

```cpp
SpinLockProvider& operator =(const SpinLockProvider&) = delete;
```

### `SpinLockProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents moving native spin-lock state.

```cpp
SpinLockProvider(SpinLockProvider&&) = delete;
```

### `operator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents move assignment of native spin-lock state.

```cpp
SpinLockProvider& operator =(SpinLockProvider&&) = delete;
```

### `Acquire`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Spins until the lock is acquired and enters the ESP-IDF critical section.

```cpp
void Acquire() noexcept
```

### `Release`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Releases the ordinary-context critical section.

```cpp
ESPressio::Platform::Synchronization::SpinLockReleaseResult Release() noexcept
```

### `AcquireFromInterrupt`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Acquires the spin lock from interrupt context.

```cpp
ESPressio::Platform::Synchronization::SpinLockAcquireResult AcquireFromInterrupt() noexcept
```

### `ReleaseFromInterrupt`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Releases the spin lock from interrupt context.

```cpp
ESPressio::Platform::Synchronization::SpinLockReleaseResult ReleaseFromInterrupt() noexcept
```

### `SpinLockContract`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Compile-time validation of the ESP-IDF SpinLock provider.

```cpp
using SpinLockContract = ESPressio::Platform::Synchronization::Detail::SpinLockProviderTraits<SpinLockProvider>;
```

