# EDP-Platform-ESP-IDF Dependency Graph

## Library Dependency Tree

```text
EDP-Platform-ESP-IDF
└── EDP-Platform [mandatory]
    └── EDP-System [mandatory, transitive]
```

## Mandatory Dependencies

- `EDP-Platform` — owns the platform-neutral capability contracts implemented by this ESP-IDF backend.
- `EDP-System` — transitive through `EDP-Platform` for Composition.

## Optional Dependencies

There are currently no optional ESPressio library dependencies.

ESP-IDF and its C++ standard atomic implementation are target facilities, not ESPressio library dependencies.
