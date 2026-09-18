# ESPressio Platform - ESP-IDF

`EDP-Platform-ESP-IDF` supplies concrete implementations for `EDP-Platform` abstractions that are specifically backed by ESP-IDF APIs or ESP-IDF extensions.

## Providers

Current ESP-IDF-owned providers:

```text
ExecutionContext
    caller-supplied static task/control storage
    priority
    processor affinity
    stack telemetry when enabled

SpinLock
    portMUX_TYPE-backed
    ordinary and interrupt-context critical-section support
```

The execution provider uses ESP-IDF's `xTaskCreateStaticPinnedToCore` extension.

FreeRTOS mutexes, semaphores and signals remain owned by `EDP-Platform-FreeRTOS` because their implementations use portable FreeRTOS APIs rather than ESP-IDF-specific APIs.

`AtomicWord32` is owned by `EDP-Platform-Portable` because its implementation is entirely Standard C++.

## Dependencies

```text
EDP-Platform-ESP-IDF
└── EDP-Platform [mandatory]
    └── EDP-System [transitive]
```

There are no optional ESPressio library dependencies.

See [DEPENDENCY_GRAPH.MD](./DEPENDENCY_GRAPH.MD).


## Documentation

- [ExecutionContext provider](./docs/EXECUTION-CONTEXT.MD)
- [SpinLock provider](./docs/SPINLOCK.MD)

## Demos

Buildable demonstrations are provided under [`./demos`](./demos).
