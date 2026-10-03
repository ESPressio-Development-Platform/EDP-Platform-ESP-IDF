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

The Mesh V1 branch manifest selects EDP-Security `mesh_v1` so the concrete provider and generic contract are mutually available before mainline integration. Other direct dependencies remain on `main`. All package versions remain `0.1.0`.

> Dependency/documentation baseline: `f7c5667e8359e29d9c2a23f4aa44b8761ca6ac60` (`mesh_v1`).

The dependency direction is acyclic: EDP-Security depends only on abstract Platform/Memory contracts and never on EDP-Platform-ESP-IDF.
