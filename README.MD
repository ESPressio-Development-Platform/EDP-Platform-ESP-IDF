# ESPressio Platform - ESP-IDF

`EDP-Platform-ESP-IDF` supplies concrete implementations for abstractions specifically backed by ESP-IDF APIs or ESP-IDF extensions.

## Providers

Current ESP-IDF-owned providers:

```text
EDP-Platform
    AtomicWord32
        native lock-free 32-bit compiler atomic operations
        exact four-byte storage when the compiler proves __atomic_always_lock_free(4, nullptr)

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

`EDP-Platform-Portable` remains the Standard C++ `AtomicWord32` implementation where `std::atomic<std::uint32_t>` can prove the required always-lock-free contract. `EDP-Platform-ESP-IDF` additionally supplies a compiler-builtin implementation whose advertised `LockFree` property is derived directly from `__atomic_always_lock_free(4, nullptr)`. This keeps the provider usable from both ESP-IDF and Arduino-ESP32 without depending on ESP-IDF-private atomic headers, while still rejecting a target that cannot prove native four-byte atomics.

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
