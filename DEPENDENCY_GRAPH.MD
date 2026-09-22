# EDP-Platform-ESP-IDF Dependency Graph

> Revalidated: 2026-09-22 — ESP-IDF RandomByteSource provider uses the existing EDP-Platform contract and introduces no new ESPressio-library dependency.\n\n> Revalidated: 2026-09-20 — V2 compliance audit and execution-demo priority migration introduced no new ESPressio-library dependency.

> Revalidated: 2026-09-19 — execution-priority contract alignment introduced no new ESPressio-library dependency.

## Library Dependency Tree

```text
EDP-Platform-ESP-IDF
├── EDP-Platform [mandatory]
│   └── EDP-System [mandatory]
└── EDP-Memory [mandatory]
    ├── EDP-System [mandatory]
    └── EDP-Platform [mandatory]
        └── EDP-System [mandatory, transitive]
```

## Mandatory Dependencies

- `EDP-Platform` — defines the ESP-IDF-owned ExecutionContext, SpinLock and RandomByteSource contracts implemented here.
- `EDP-Memory` — defines the MemoryResource contract implemented by the ESP-IDF heap-capability providers.

ESP-IDF and its FreeRTOS extensions are target facilities, not ESPressio-library dependencies.

## Optional Dependencies

`EDP-Platform-ESP-IDF` has no optional ESPressio-library dependencies.

## Dependency direction

The graph remains acyclic because `EDP-Memory` depends only on abstract `EDP-Platform`, never on this concrete provider repository.
