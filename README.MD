# ESPressio Platform - ESP-IDF

`EDP-Platform-ESP-IDF` supplies ESP-IDF-environment concrete implementations for abstractions defined by `EDP-Platform`.

## Contents

- [AtomicWord32 Provider](./docs/ATOMIC-WORD32.MD)  
  Supplies the lock-free four-byte `AtomicWord32` capability required by Platform read-mostly concurrency facilities such as `ConcurrentSnapshot`.

## Dependencies

```text
EDP-Platform-ESP-IDF
└── EDP-Platform [mandatory]
    └── EDP-System [mandatory, transitive]
```

There are currently no optional ESPressio library dependencies.

See [DEPENDENCY_GRAPH.MD](./DEPENDENCY_GRAPH.MD) for the maintained dependency graph.

## Demos

The `AtomicWord32` provider demonstration is available under [`./demos/atomic-word-basic`](./demos/atomic-word-basic) for:

- Arduino IDE;
- PlatformIO Arduino;
- PlatformIO ESP-IDF.

The package remains an ESP-IDF-owned concrete backend. The policy-required cross-framework PlatformIO demo disables only PlatformIO's compatibility filter for that demo project; it does not alter the package's declared framework ownership or recommended application selection.

## Development Status

Only Platform capabilities with an actual ESP-IDF concrete responsibility are added here. The current implementation is intentionally limited to the atomic-word prerequisite required by synchronized Clock work.
