# EDP-Platform-ESP-IDF Dependency Graph

> Revalidated: 2026-09-22 — AES-256-GCM concrete provider adds mandatory EDP-Security contract dependency; graph remains acyclic.

> Revalidated: 2026-09-22 — ESP-IDF RandomByteSource provider uses the existing EDP-Platform contract and introduces no new ESPressio-library dependency.

> Revalidated: 2026-09-20 — V2 compliance audit and execution-demo priority migration introduced no new ESPressio-library dependency.

> Revalidated: 2026-09-19 — execution-priority contract alignment introduced no new ESPressio-library dependency.

## Library Dependency Tree

```text
EDP-Platform-ESP-IDF
├── EDP-Platform [mandatory]
│   └── EDP-System [mandatory]
├── EDP-Memory [mandatory]
│   ├── EDP-System [mandatory]
│   └── EDP-Platform [mandatory]
│       └── EDP-System [mandatory, transitive]
└── EDP-Security [mandatory]
    ├── EDP-System [mandatory, transitive]
    ├── EDP-Platform [mandatory, transitive]
    └── EDP-Memory [mandatory, transitive]
```

## Mandatory Dependencies

- `EDP-Platform` — defines the ESP-IDF-owned ExecutionContext, SpinLock and RandomByteSource contracts implemented here.
- `EDP-Memory` — defines the MemoryResource contract implemented by the ESP-IDF heap-capability providers and ByteOperations contract consumed by the AES-GCM provider.
- `EDP-Security` — defines the AuthenticatedCrypto capability and key-operation contract implemented by the AES-256-GCM provider.

ESP-IDF and its FreeRTOS extensions are target facilities, not ESPressio-library dependencies.

## Optional Dependencies

`EDP-Platform-ESP-IDF` has no optional ESPressio-library dependencies.

## Dependency direction

The graph remains acyclic because `EDP-Security` depends on abstract `EDP-Platform` and `EDP-Memory`, never on this concrete provider repository.
