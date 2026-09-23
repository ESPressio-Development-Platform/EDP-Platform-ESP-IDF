# Dependency Contracts

EDP-Platform-ESP-IDF depends on **EDP-Platform**, **EDP-Memory** and **EDP-Security**.

## EDP-Platform providers

### ExecutionContext

Offers the Platform `ExecutionContext` capability with caller-supplied storage, priority support and processor-affinity support. Backing sizes/alignments are derived from ESP-IDF/FreeRTOS native types and validated through `ExecutionContextProviderTraits`.

### SpinLock

Offers Platform `SpinLock` with `SpinLockSupportsInterruptContext=true`, backed by `portMUX_TYPE`.

### RandomByteSource

Offers the shared Platform `RandomByteSource` capability and sets the `CryptographicallySuitable` feature. This provider therefore satisfies Security's cryptographic randomness requirement.

## EDP-Memory providers

Internal, external/PSRAM-only and external-preferred resource providers offer the Memory `MemoryResource` capability. External-preferred is a policy provider that tries PSRAM first and falls back to internal memory.

## EDP-Security provider

`Aes256GcmProvider<TByteOperationsProvider>` offers `AuthenticatedCrypto` with:

- algorithm: AES-256-GCM;
- key bytes: 32;
- default DataProtection nonce bytes: 12;
- tag bytes: 16;
- key-access mode: RawMaterial.

Its Composition Contract requires **exactly one external EDP-Memory ByteOperations provider**. It also validates the supplied raw-key provider at each operation boundary through EDP-Security's internal provider traits.

## Dependency direction

There is no cycle: abstract EDP-Security depends on abstract Platform/Memory; this concrete repository depends on EDP-Security only to implement its concrete cryptographic provider.

> Dependency contract audit baseline: `59fce5555e87517901afe33b708a0bad2fe4502b` (`main`).
