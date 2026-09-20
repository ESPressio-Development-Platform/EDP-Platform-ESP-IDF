# ESPressio Platform - ESP-IDF

`EDP-Platform-ESP-IDF` supplies concrete implementations for abstractions specifically backed by ESP-IDF APIs or ESP-IDF extensions.

## Providers

Current ESP-IDF-owned providers:

```text
EDP-Platform
    AtomicWord32
        native lock-free 32-bit compiler/ESP-IDF atomic operations
        exact four-byte storage when ESP-IDF advertises HAS_ATOMICS_32

    ExecutionContext
        caller-supplied static task/control storage
        priority
        processor affinity
        stack telemetry when enabled

    SpinLock
        portMUX_TYPE-backed
        ordinary and interrupt-context critical-section support

EDP-Memory
    InternalMemoryResource
    ExternalMemoryResource
    ExternalPreferredMemoryResource
```

The Memory providers use ESP-IDF heap-capability APIs. ByteOperations remains owned by `EDP-Platform-Portable` because its concrete implementation uses Standard C/C++ byte primitives.

FreeRTOS mutexes, semaphores and signals remain owned by `EDP-Platform-FreeRTOS` because their implementations use portable FreeRTOS APIs rather than ESP-IDF-specific APIs.

`EDP-Platform-Portable` remains the Standard C++ `AtomicWord32` implementation where `std::atomic<std::uint32_t>` can prove the required always-lock-free contract. `EDP-Platform-ESP-IDF` additionally supplies a native ESP-IDF implementation when `HAS_ATOMICS_32` is available, allowing ESP32-family targets to satisfy the same Platform capability without relying on libstdc++'s `is_always_lock_free` trait.

## Dependencies

```text
EDP-Platform-ESP-IDF
├── EDP-Platform [mandatory]
│   └── EDP-System [mandatory]
└── EDP-Memory [mandatory]
    ├── EDP-System [mandatory]
    └── EDP-Platform [mandatory]
```

There are no optional ESPressio-library dependencies.

See [DEPENDENCY_GRAPH.MD](./DEPENDENCY_GRAPH.MD).

## Documentation

- [ExecutionContext Provider](./docs/EXECUTION-CONTEXT.MD) — static task lifecycle, storage, affinity, waits and telemetry.
- [SpinLock Provider](./docs/SPINLOCK.MD) — `portMUX_TYPE` critical-section provider and ISR behavior.
- [Memory Resources](./docs/MEMORY-RESOURCES.MD) — internal, external/PSRAM-only, and external-preferred/internal-fallback raw resources.

## Demos

Buildable demonstrations are provided under [`./demos`](./demos). Arduino surfaces target Arduino-ESP32, where the underlying ESP-IDF APIs are available.
