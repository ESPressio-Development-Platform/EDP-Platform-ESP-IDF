# ESP-IDF Random Byte Source

## Purpose

`RandomByteSourceProvider` satisfies the Platform `RandomByteSource` capability using ESP-IDF's system random facility.

It is owned by `EDP-Platform-ESP-IDF` because the implementation directly calls an ESP-IDF API.

## Composition advertisement

The provider advertises:

- `RandomByteSource`;
- `RandomByteSourceFeature::CryptographicallySuitable`.

Standalone consumers may qualify cryptographic randomness with the Platform `CryptographicRandomByteSourceRequirement`. Provider Contracts in other Domains should reuse `CryptographicRandomByteSourceConstraint` inside an explicitly scoped external `Requirement`, avoiding any ESP-IDF-specific vocabulary in those domains.

## Operation

`FillRandomBytes` writes directly into caller-owned storage.

- zero-byte requests succeed without dereferencing the destination;
- non-zero requests require a non-null destination;
- no heap allocation is performed;
- the provider retains no random output or historical diagnostic state.

## ESP-IDF source

The provider delegates to `esp_fill_random`.

The exact entropy guarantees therefore follow the ESP-IDF target/runtime contract. Consumers should use this provider only on supported ESP-IDF/Arduino-ESP32 targets where that facility is available.

## Security boundary

This provider supplies unpredictable bytes only. It does not own Security nonce policy, key generation policy, session identity, replay state, or cryptographic algorithm selection.
