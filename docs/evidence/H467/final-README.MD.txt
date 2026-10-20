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

    DigitalSignature
        canonical low-S P-256 ECDSA/SHA-256 via Mbed TLS
        Bootstrap-owned key source + EDP-Platform cryptographic random source
        optional H363 development-lab P-256 key source in ordinary NVS
        NOT hardware-hardened; NOT independently certified Clock election

    SecretZeroization
        mbedtls_platform_zeroize-backed

    DeploymentKeySource
        fixed caller-provisioned application key and deployment PSK
        allocation-free HKDF-SHA-256 orchestration over Mbed TLS SHA-256

EDP-Memory
    InternalMemoryResource
    ExternalMemoryResource
    ExternalPreferredMemoryResource
```

The Memory providers use ESP-IDF heap-capability APIs. ByteOperations remains owned by `EDP-Platform-Portable` because its concrete implementation uses Standard C/C++ byte primitives.

FreeRTOS mutexes, semaphores and signals remain owned by `EDP-Platform-FreeRTOS` because their implementations use portable FreeRTOS APIs rather than ESP-IDF-specific APIs.

The explicit-include native `AtomicWord32Provider` supports only ESP32 and ESP32-S3 with native S32C1I and complete Word storage in internal DRAM. Unsupported targets are rejected at compile time; unsupported storage aborts construction before its first store. ESP32-S2, external RAM, RTC memory and RISC-V are outside this provider's contract. See [native atomic contract](./docs/ATOMIC-WORD-32.MD).

Consumers which need only the ESP-IDF SpinLock concrete may include the narrow `<ESPressio_Platform_ESP_IDF_SpinLock.hpp>` entry point. It imports only `SpinLockProvider` and avoids pulling the broad execution/memory/randomness/security provider umbrella into narrow synchronization consumers.

## Dependencies

```text
EDP-Platform-ESP-IDF
├── EDP-Platform [mandatory]
│   └── EDP-System [mandatory, transitive]
├── EDP-Memory [mandatory]
│   ├── EDP-System [mandatory, transitive]
│   ├── EDP-Platform [mandatory, transitive; dependencies expanded above]
│   └── EDP-BoundedTopology [mandatory, transitive]
├── EDP-Security [mandatory]
│   ├── EDP-System [mandatory, transitive]
│   ├── EDP-Platform [mandatory, transitive; dependencies expanded above]
│   └── EDP-Memory [mandatory, transitive; dependencies expanded above]
├── EDP-Clock [mandatory]
│   ├── EDP-System [mandatory, transitive]
│   └── EDP-Platform [mandatory, transitive; dependencies expanded above]
├── EDP-Persistence [mandatory]
│   └── EDP-System [mandatory, transitive]
└── EDP-System [mandatory]
```

Every mandatory transitive edge is listed in the linked graph; repeated tree nodes refer to the first expanded occurrence. Concrete providers are selected by application Bootstrap.


See [DEPENDENCY_GRAPH.MD](./DEPENDENCY_GRAPH.MD) for the complete graph and ownership boundaries.

## Documentation

- [Development-lab P-256 key source via ordinary EDP-Persistence storage](./docs/DEVELOPMENT-P256-NVS-KEYS.MD) — explicit one-time provision/reboot readback, independent board keys; **no hardware security or Clock Reference authority**.
- [P-256 ECDSA DigitalSignature provider](./docs/EDP-SIGNATURE-IMPLEMENTATION.MD) — genuine Mbed TLS signature and low-S verification with host regression tests; **not** yet per-device key provisioning, durable voter signatures or Clock Reference authority.

- [ExecutionContext Provider](./docs/EXECUTION-CONTEXT.MD) — static task lifecycle, storage, affinity, waits and telemetry.
- [SpinLock Provider](./docs/SPINLOCK.MD) — `portMUX_TYPE` critical-section provider and ISR behavior.
- [Random Byte Source](./docs/RANDOMNESS.MD) — ESP-IDF cryptographic random-byte provider and ownership semantics.
- [AES-256-GCM Security Provider](./docs/AES-256-GCM.MD) — Mbed TLS authenticated-crypto provider, segmented AAD handling and authentication-before-publication semantics.
- [Deployment-Key Source and Secret Zeroization](./docs/DEPLOYMENT-KEY-SOURCE.MD) — bounded root provisioning, allocation-free HKDF-SHA-256, zeroization, Composition and lifecycle semantics.
- [Memory Resources](./docs/MEMORY-RESOURCES.MD) — internal, external/PSRAM-only, and external-preferred/internal-fallback raw resources.

## Demos

Buildable demonstrations are provided under [`./demos`](./demos), including end-to-end deployment-key provisioning and generic key lifecycle integration. Arduino surfaces target Arduino-ESP32, where the underlying ESP-IDF APIs are available.

## C++ Language Baseline

ESPressio requires C++20. PlatformIO-compatible ESP32 examples should use a maintained C++20-capable PIOArduino platform package rather than the outdated Arduino-ESP32 package bundled by stock PlatformIO. Native ESP-IDF builds must likewise use a C++20-capable toolchain.

H433: native cryptographic randomness requires [explicit SDK entropy ownership](docs/EXPLICIT-ENTROPY-OWNERSHIP.MD). Default nonzero requests now refuse; migrate Bootstrap before use.


## H467 native canonical timebase

The stateless `Timing::EspTimerMonotonicTimebaseProvider` offers the existing SDK
microsecond counter through EDP-Clock's existing MonotonicTimebase capability.
Bootstrap supplies it to one canonical MonotonicClock; no timer, clock service,
task, lock, heap allocation, synchronization estimator or counter reset is added.
The mandatory native-provider to abstract EDP-Clock contract edge is intentional.
EDP-Clock depends only on EDP-System and abstract EDP-Platform, so there is no cycle.
All six native direct dependencies and their complete closure are shown above.
Version remains 0.1.0. See [the specification contract and limits](docs/NATIVE-MONOTONIC-TIMEBASE.MD)
and [the three-form example](demos/native-monotonic-timebase/README.MD).
