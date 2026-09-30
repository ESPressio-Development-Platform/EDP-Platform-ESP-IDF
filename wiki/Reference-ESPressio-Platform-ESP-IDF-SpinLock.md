# src/ESPressio_Platform_ESP_IDF_SpinLock.hpp

**Primary classification:** PUBLIC NARROW ENTRY POINT

[Open current source](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/main/src/ESPressio_Platform_ESP_IDF_SpinLock.hpp)

## Purpose

Provides a minimal public include surface for consumers which need only the existing ESP-IDF `SpinLockProvider` without importing the broad ESP-IDF provider umbrella.

## Direct include

- `synchronization/SpinLockProvider.hpp`

## Declarations and runtime state

This aggregation header declares no new Type, capability, provider, object, state, storage or runtime behavior. The re-exported `SpinLockProvider` remains the single authoritative ESP-IDF concrete for `EDP-Platform::Synchronization::SpinLock`.

Using this narrow entry point does not alter Composition identity, provider lifetime, interrupt-context support, memory usage or dependency direction. It exists only to make the existing provider discoverable through ordinary library include resolution without importing unrelated execution, memory, randomness or Security provider surfaces.
