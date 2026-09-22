# ESPressio Platform - ESP-IDF

`EDP-Platform-ESP-IDF` supplies concrete implementations for abstractions specifically backed by ESP-IDF APIs or ESP-IDF extensions.

## Providers

Current ESP-IDF-owned providers:

```text
EDP-Platform
    ExecutionContext
        caller-supplied static task/control storage
        priority
        processor affinity
        stack telemetry when enabled

    SpinLock
        portMUX_TYPE-backed
        ordinary and interrupt-context critical-section support

    RandomByteSource
        ESP-IDF system random source
        cryptographically suitable

EDP-Security
    AuthenticatedCrypto
        AES-256-GCM through Mbed TLS
        raw-key access

EDP-Memory
    InternalMemoryResource
    ExternalMemoryResource
    ExternalPreferredMemoryResource
```

The Memory providers use ESP-IDF heap-capability APIs. ByteOperations remains owned by `EDP-Platform-Portable` because its concrete implementation uses Standard C/C++ byte primitives.

FreeRTOS mutexes, semaphores and signals remain owned by `EDP-Platform-FreeRTOS` because their implementations use portable FreeRTOS APIs rather than ESP-IDF-specific APIs.

`AtomicWord32` remains owned by `EDP-Platform-Portable` only on targets where the Standard C++ implementation can prove the required always-lock-free contract. ESP-IDF's target-specific compare-and-set facilities are not exposed here as a generic `AtomicWord32` provider because their lock-free behavior can depend on storage location.

## Dependencies

```text
EDP-Platform-ESP-IDF
├── EDP-Platform [mandatory]
│   └── EDP-System [mandatory]
├── EDP-Memory [mandatory]
│   ├── EDP-System [mandatory]
│   └── EDP-Platform [mandatory]
└── EDP-Security [mandatory]
    ├── EDP-System [mandatory, transitive]
    ├── EDP-Platform [mandatory, transitive]
    └── EDP-Memory [mandatory, transitive]
```

There are no optional ESPressio-library dependencies.

See [DEPENDENCY_GRAPH.MD](./DEPENDENCY_GRAPH.MD).

## Documentation

- [ExecutionContext Provider](./docs/EXECUTION-CONTEXT.MD) — static task lifecycle, storage, affinity, waits and telemetry.
- [SpinLock Provider](./docs/SPINLOCK.MD) — `portMUX_TYPE` critical-section provider and ISR behavior.
- [Random Byte Source](./docs/RANDOMNESS.MD) — ESP-IDF cryptographic random-byte provider and ownership semantics.
- [AES-256-GCM Security Provider](./docs/AES-256-GCM.MD) — Mbed TLS authenticated-crypto provider, segmented AAD handling and authentication-before-publication semantics.
- [Memory Resources](./docs/MEMORY-RESOURCES.MD) — internal, external/PSRAM-only, and external-preferred/internal-fallback raw resources.

## Demos

Buildable demonstrations are provided under [`./demos`](./demos). Arduino surfaces target Arduino-ESP32, where the underlying ESP-IDF APIs are available.

## C++ Language Baseline

ESPressio requires C++20. PlatformIO-compatible ESP32 examples should use a maintained C++20-capable PIOArduino platform package rather than the outdated Arduino-ESP32 package bundled by stock PlatformIO. Native ESP-IDF builds must likewise use a C++20-capable toolchain.
