# EDP-Platform-ESP-IDF Dependency Graph

> Revalidated: 2026-10-03 — provisioned DeploymentKeySource and SecretZeroization providers extend the existing mandatory EDP-Security contract edge. EDP-BoundedTopology is now shown as the mandatory transitive dependency introduced by EDP-Memory; no new direct package edge is created here.

> Revalidated: 2026-09-23 — Composition Framework enhancement migration changes compile-time provider/requirement metadata and validation only; no ESPressio-library dependency edge changed.

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
│   ├── EDP-Platform [mandatory]
│   │   └── EDP-System [mandatory, transitive]
│   └── EDP-BoundedTopology [mandatory]
└── EDP-Security [mandatory]
    ├── EDP-System [mandatory, transitive]
    ├── EDP-Platform [mandatory, transitive]
    ├── EDP-Memory [mandatory, transitive]
    └── EDP-BoundedTopology [mandatory, transitive through EDP-Memory]
```

## Mandatory Dependencies

- `EDP-Platform` — defines the ESP-IDF-owned ExecutionContext, SpinLock and RandomByteSource contracts implemented here.
- `EDP-Memory` — defines the MemoryResource contract implemented by the ESP-IDF heap-capability providers and ByteOperations contract consumed by the AES-GCM provider.
- `EDP-Security` — defines the AuthenticatedCrypto, SecretZeroization and DeploymentKeySource capabilities implemented here, plus the generic key lifecycle and derivation request contracts consumed by the provisioned source.

`EDP-BoundedTopology` is required transitively by `EDP-Memory`; no production source in this repository includes it directly.

ESP-IDF and its FreeRTOS extensions are target facilities, not ESPressio-library dependencies.

## Optional Dependencies

`EDP-Platform-ESP-IDF` has no optional ESPressio-library dependencies.

## Dependency direction

The graph remains acyclic because `EDP-Security` depends on abstract `EDP-Platform`, `EDP-Memory` and their acyclic dependencies, never on this concrete provider repository. The concrete deployment-key source points toward the generic EDP-Security contract; EDP-Security does not point back toward this repository.

The narrow `ESPressio_Platform_ESP_IDF_SpinLock.hpp` public entry point introduces no additional dependency edge; it is only a reduced include surface over the existing ESP-IDF SpinLock provider and its existing `EDP-Platform` contract.
