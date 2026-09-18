# EDP-Platform-ESP-IDF Dependency Graph

## Library Dependency Tree

```text
EDP-Platform-ESP-IDF
└── EDP-Platform [mandatory]
    └── EDP-System [transitive]
```

## Mandatory Dependencies

- `EDP-Platform` — defines the Platform ExecutionContext and SpinLock capabilities implemented by this ESP-IDF backend.

## Optional Dependencies

`EDP-Platform-ESP-IDF` has no optional ESPressio library dependencies.

ESP-IDF and its FreeRTOS extensions are target facilities, not ESPressio-library dependencies.
