# Dependency Contracts

EDP-Platform-ESP-IDF has mandatory direct package dependencies on **EDP-Platform**, **EDP-Memory** and **EDP-Security**. **EDP-BoundedTopology** is mandatory transitively through EDP-Memory.

## EDP-Platform contracts

### ExecutionContext

Offers caller-static task/control storage, priority support and processor affinity. Native sizes and alignments are derived from ESP-IDF/FreeRTOS Types and validated through provider traits.

### SpinLock

Offers `SpinLock` with interrupt-context support backed by `portMUX_TYPE`.

### RandomByteSource

Offers the shared `RandomByteSource` capability with `CryptographicallySuitable`. This qualifies it for EDP-Security consumers requiring cryptographic randomness.

## EDP-Memory contracts

Internal, external-only and external-preferred providers offer `MemoryResource`. AES-GCM requires exactly one external-domain `ByteOperations` provider for bounded copies and scratch erasure.

EDP-Memory's EDP-BoundedTopology dependency is transitive here; no production header in this repository directly includes EDP-BoundedTopology.

## EDP-Security contracts

### Authenticated crypto

`Aes256GcmProvider<TByteOperationsProvider>` offers AES-256-GCM with a 32-byte key, 12-byte default data-protection nonce, 16-byte tag and RawMaterial key access.

### Secret zeroization

`SecretZeroizationProvider` offers the shared `SecretZeroization` capability and implements the exact `noexcept void SecureZeroize(MutableByteView)` provider trait.

### Deployment-key source

`ProvisionedDeploymentKeySourceProvider` consumes:

- `DeploymentKeySource` capability/property vocabulary;
- `DeploymentKeyDerivationRequest` and result vocabulary;
- `WriteDeploymentKeyDerivationInfo` canonical encoder;
- `SecretZeroizationTraits` for its same-domain dependency.

It offers HKDF-SHA-256 with application key as salt, deployment PSK as input key material, 32-byte output and a template-selected context capacity.

EDP-Security owns the generic `DeploymentKeyProvider` which consumes this concrete source. That direction is an application/provider binding, not a reverse package dependency.

## Target facilities

ESP-IDF, FreeRTOS extensions and Mbed TLS are target SDK facilities, not ESPressio-library dependencies.

## Branch baseline

The current native package selects reintegrated Security main (a7482cbc5f455d407860ad36a2b1589441674c94) and Platform feature/profiling-v1 (5d1ff9c79e230d7f7e243e6d35b5e05916d9c501) for the additive typed binding vocabulary. Other production dependencies retain their main selections. All versions remain 0.1.0. The AES demo selects only Portable's existing narrow ByteOperations capability; it introduces no production package edge or host synchronization/atomic provider.

> Historical pre-reintegration dependency/documentation baseline: `f7c5667e8359e29d9c2a23f4aa44b8761ca6ac60` (`mesh_v1`).

The dependency direction is acyclic: EDP-Security depends only on abstract Platform/Memory contracts and never on EDP-Platform-ESP-IDF.


## Arduino Clock discovery correction

The native timebase consumes the existing public ESPressio_Clock.hpp entry point. Arduino library.properties now declares the same Clock dependency already in library.json. This resolves Arduino IDE subdirectory-header discovery for broad native headers without a new capability, timer or runtime behavior. Source/metadata correction: a5454b245ffbd5e2edff19db91315357d08c2815.

Four additional execution/AES Arduino IDE builds pass on ESP32/S3 (RPI400 20261120T113159-7254e373). Four SDK root builds with exact updated dependency heads pass (AI02 20261120T113247-aa8b6bf3); native/Security GCC/Clang Debug/Release owner regressions pass (20261120T113200-d02fab0f). Original discovery failures are retained in the Policy handoff.
