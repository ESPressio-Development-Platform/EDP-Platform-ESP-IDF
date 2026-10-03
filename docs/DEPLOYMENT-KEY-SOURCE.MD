# Deployment-Key Source and Secret Zeroization

## Purpose and ownership

EDP-Security defines the platform-neutral Mesh V1 deployment-key contracts. `EDP-Platform-ESP-IDF` owns the initial ESP-IDF concrete implementations:

- `SecretZeroizationProvider`, backed by `mbedtls_platform_zeroize`;
- `ProvisionedDeploymentKeySourceProvider`, a bounded RAM-backed source for caller-provisioned application and deployment root secrets.

The generic `ESPressio::Security::DeploymentKeyProvider` remains in EDP-Security. It consumes this source to derive and retain the six purpose-separated working keys used by Mesh V1.

This split keeps root loading and storage platform-owned while keeping key purpose, generation, rotation and retirement semantics platform-neutral.

## Root-secret contract

The initial provisioned source accepts exactly two caller-owned byte ranges at Bootstrap:

1. the application-wide symmetric key;
2. the operator/deployment pre-shared key.

Both exact sizes are template parameters. They therefore affect object layout and are fixed at compile time for one application topology.

On successful `Initialize`, both roots are copied into private fixed storage. The caller retains ownership of the supplied buffers and remains responsible for erasing its copies. The demo explicitly uses the selected zeroizer immediately after provisioning.

The source exposes no root accessor. Its only cryptographic operation is the generic `Derive(request, output)` contract.

Root strength, generation, transport to the device and authority to replace roots remain provisioning/application responsibilities. The provider validates exact sizes; it does not attempt heuristic entropy testing.

## Exact Mesh V1 derivation

The provider advertises the following Composition properties:

- algorithm: `HkdfSha256`;
- input order: application key as HKDF salt, deployment PSK as input key material;
- output: 32 bytes;
- application-context capacity: the template-selected fixed capacity.

Derivation is:

1. HKDF-Extract: `HMAC-SHA-256(application key, deployment PSK)`;
2. canonical information encoding through EDP-Security's `WriteDeploymentKeyDerivationInfo`;
3. one-block HKDF-Expand: `HMAC-SHA-256(PRK, canonical information || 0x01)`.

The canonical information binds the derivation version, Mesh security suite, wire-protocol version, purpose, direction, stable key identifier, immutable generation and application context.

Changing either root, the application context, purpose, wire version, key identifier or generation changes the resulting working-key domain.

## Why the implementation uses the SHA-256 streaming API

The provider deliberately does not call the generic Mbed TLS `mbedtls_hkdf` helper. In the ESP-IDF Mbed TLS implementation, its generic message-digest setup may allocate internal HMAC state.

Instead, the provider implements the small HKDF orchestration around Mbed TLS's low-level SHA-256 streaming primitive. SHA-256 itself remains supplied by Mbed TLS; this repository does not implement a hash primitive.

The resulting production path has no provider-owned heap allocation and no input-dependent storage growth.

## Deterministic memory

One source instance owns exactly:

~~~text
ApplicationKeyBytes + DeploymentPskBytes
~~~

secret root bytes, plus a zeroizer pointer, lifecycle flag and normal target ABI padding.

Each derivation uses bounded stack storage for:

- at most `DeploymentKeyDerivationInfoFixedBytes + ApplicationContextCapacity` canonical information bytes;
- one 32-byte pseudorandom key;
- one unpublished 32-byte output;
- a 64-byte HMAC key block and 32-byte inner digest;
- Mbed TLS SHA-256 contexts.

Public constants expose the root sizes, total owned root bytes, context capacity and maximum information size.

The caller supplies the final 32-byte output buffer. A derived key is copied into that buffer only after Extract and Expand both succeed.

## Zeroization and failure preservation

`SecretZeroizationProvider::SecureZeroize` erases a valid non-empty mutable range through `mbedtls_platform_zeroize`.

The key source erases:

- HMAC key blocks and inner digests;
- HKDF pseudorandom and unpublished output buffers;
- the canonical information buffer;
- caller output after a cryptographic provider failure;
- both retained roots during `Destroy` and destruction.

Invalid requests, invalid output descriptors and an unprovisioned source do not overwrite otherwise valid caller output. A failure after cryptographic processing begins returns `ProviderFailure` and clears the exact 32-byte output.

`Destroy` is idempotent. After it completes, `Derive` reports `RootSecretUnavailable`, and the source may be provisioned again.

## Initialization outcomes

`ProvisionedDeploymentKeySourceInitializationResult` reports:

- `Succeeded` — both exact-size roots were copied;
- `AlreadyInitialized` — live roots already exist;
- `InvalidApplicationKey` — the application-key view is invalid or has the wrong size;
- `InvalidDeploymentPsk` — the PSK view is invalid or has the wrong size.

Initialization validates both views before copying either root.

## Composition contract

`SecretZeroizationProvider` offers the shared EDP-Security `SecretZeroization` capability.

`ProvisionedDeploymentKeySourceProvider` offers the exclusive EDP-Security `DeploymentKeySource` capability and requires exactly one same-domain `SecretZeroization` provider.

Its lifecycle metadata requires the source to initialize after, and shut down before, the selected zeroizer. Bootstrap owns both provider objects and must keep the zeroizer alive for the entire source lifetime.

A typical topology binds:

~~~cpp
using Zeroizer =
    ESPressio::Platform::ESPIDF::Security::SecretZeroizationProvider;

using Source =
    ESPressio::Platform::ESPIDF::Security::ProvisionedDeploymentKeySourceProvider<
        Zeroizer,
        32U,
        32U,
        16U
    >;

using Keys =
    ESPressio::Security::DeploymentKeyProvider<Source, Zeroizer, 16U>;
~~~

The source context capacity must be at least the capacity selected for the generic deployment-key provider.

A future secure-element, eFuse-backed or other opaque-root implementation can replace the provisioned RAM source by offering the same EDP-Security capability and properties. Generic key lifecycle code must not depend on this concrete class.

## Concurrency and execution context

Operations on one source instance are caller-serialized. The provider does not own a lock, worker, queue or task and does not wait.

Provisioning and derivation are Bootstrap/application-context operations, not ISR operations.

## Target configuration

The target must provide Mbed TLS SHA-256 and `mbedtls_platform_zeroize`, as supported by the ESP-IDF/Arduino-ESP32 environments used by this repository.

The provider does not require `CONFIG_MBEDTLS_HKDF_C`, because it does not use the allocation-capable generic HKDF helper.

## Validation

Host coverage verifies:

- provider capability/property contracts;
- exact provisioning and lifecycle results;
- six fixed independent HKDF-SHA-256 known-answer vectors;
- root ordering and context/purpose/generation separation;
- failure-to-result mapping and output clearing;
- explicit and destructor-driven root zeroization;
- non-copyable/non-movable ownership.

The buildable `deployment-key-source-basic` demo integrates the concrete providers with EDP-Security's generic `DeploymentKeyProvider` across Arduino IDE, PlatformIO Arduino and PlatformIO ESP-IDF surfaces.
