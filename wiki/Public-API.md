# Public API

Concrete providers implement abstract Platform, Memory and Security capabilities. Exact declarations remain authoritative in exported headers and are exhaustively linked from [Reference Index](Reference-Index.md).

## Entry points

`<ESPressio_Platform_ESP_IDF.hpp>` imports every provider family.

`<ESPressio_Platform_ESP_IDF_SpinLock.hpp>` is the narrow entry point for consumers requiring only `SpinLockProvider`.

## Platform and Memory

ExecutionContext uses static ESP-IDF task creation and supports processor affinity. SpinLock wraps `portMUX_TYPE`. RandomByteSource uses `esp_fill_random` and advertises cryptographic suitability.

Memory resources expose internal-only, external/PSRAM-only and external-preferred allocation policies.

## Authenticated crypto

`Aes256GcmProvider<TByteOperationsProvider>` advertises AES-256-GCM, 32-byte keys, a 12-byte default data-protection nonce, 16-byte tags and RawMaterial access. Decrypt authenticates before publishing plaintext.

## Secret zeroization

`SecretZeroizationProvider` offers `SecureZeroize(MutableByteView) noexcept` through Mbed TLS's optimization-resistant erasure primitive.

## Provisioned deployment-key source

`ProvisionedDeploymentKeySourceInitializationResult` is the public provisioning result vocabulary.

`ProvisionedDeploymentKeySourceProvider<TZeroizer, ApplicationKeyBytes, DeploymentPskBytes, ApplicationContextCapacity>`:

- owns exact-size private root copies;
- offers the generic root-hiding `DeploymentKeySource` capability;
- derives exact 32-byte Mesh V1 working keys;
- exposes deterministic resource constants;
- supports explicit idempotent `Destroy` and reprovisioning;
- is non-copyable and non-movable.

Public operations are `Initialize`, `Destroy` and generic `Derive`. No operation exposes either root.

The application normally combines this source and zeroizer with EDP-Security's `DeploymentKeyProvider` rather than invoking six derivations manually.
