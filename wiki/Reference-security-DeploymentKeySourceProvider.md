# src/security/DeploymentKeySourceProvider.hpp

**Primary classification:** PUBLIC PROVIDER / EXTENSION API

**Source baseline:** `524a881b9ed27a86c5b886429558242d88962b70`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/524a881b9ed27a86c5b886429558242d88962b70/src/security/DeploymentKeySourceProvider.hpp)

## Purpose

This header supplies the initial ESP-IDF root-secret binding for EDP-Security's Mesh V1 deployment-key contracts.

`ProvisionedDeploymentKeySourceProvider` owns fixed caller-provisioned root copies in private RAM and exposes only root-hiding derivation. It performs the exact Suite V1 HKDF-SHA-256 transcript without using Mbed TLS's allocation-capable generic message-digest/HKDF setup.

## Direct includes

- `array`
- `cstddef`
- `cstdint`
- `type_traits`
- `mbedtls/sha256.h`
- `ESPressio_Security.hpp`

## Composition offer and requirement

The provider belongs to `ESPressio::Security::Domain` and offers the exclusive `DeploymentKeySource` capability with these properties:

| Property | Value |
|---|---|
| `DeploymentKeySourceAlgorithm` | `HkdfSha256` |
| `DeploymentKeySourceInputOrder` | application key as salt; deployment PSK as input key material |
| `DeploymentKeySourceOutputBytes` | `DeploymentWorkingKeyBytes` (32) |
| `DeploymentKeySourceApplicationContextCapacity` | template parameter `TApplicationContextCapacity` |

It requires exactly one same-domain `SecretZeroization` provider, initializes after that provider and shuts down before it.

Bootstrap owns both objects. The borrowed zeroizer must outlive the source.

## Public result vocabulary

### `ProvisionedDeploymentKeySourceInitializationResult`

**Classification:** PUBLIC API

~~~cpp
enum class ProvisionedDeploymentKeySourceInitializationResult : std::uint8_t
~~~

- `Succeeded` — both validated roots were copied into private storage.
- `AlreadyInitialized` — the instance already owns live roots; existing roots are preserved.
- `InvalidApplicationKey` — the view is structurally invalid or does not contain exactly `TApplicationKeyBytes`.
- `InvalidDeploymentPsk` — the view is structurally invalid or does not contain exactly `TDeploymentPskBytes`.

Both views are validated before either root is copied.

## Provider requirement alias

### `Detail::ProvisionedDeploymentKeySourceZeroizationRequirement`

**Classification:** INTERNAL PROVIDER CONTRACT

~~~cpp
using ProvisionedDeploymentKeySourceZeroizationRequirement =
    Framework::Requirement<
        ESPressio::Security::SecretZeroization,
        Framework::RequirementScope::SameDomain,
        Framework::ExactlyProviders<1U>
    >;
~~~

The alias centralizes the exact requirement reused by the contract's cardinality and lifecycle-order entries.

## Provider Type

### `ProvisionedDeploymentKeySourceProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

~~~cpp
template<
    class TZeroizer,
    std::size_t TApplicationKeyBytes,
    std::size_t TDeploymentPskBytes,
    std::size_t TApplicationContextCapacity
>
class ProvisionedDeploymentKeySourceProvider final;
~~~

Template parameters:

- `TZeroizer` — concrete same-domain provider satisfying `SecretZeroizationTraits`. The source borrows it; no ownership transfer occurs.
- `TApplicationKeyBytes` — exact application-key input and retained storage bytes. It must be greater than zero.
- `TDeploymentPskBytes` — exact deployment-PSK input and retained storage bytes. It must be greater than zero.
- `TApplicationContextCapacity` — maximum application-context bytes accepted in one derivation request. It must be in [1, 65535] and contributes directly to bounded stack storage.

The Type is final, non-copyable and non-movable. Provider substitution occurs by selecting another Type offering the generic EDP-Security capability.

## Private compile-time vocabulary

### `ZeroizationContract`

**Classification:** INTERNAL PROVIDER CONTRACT

Instantiates `SecretZeroizationTraits<TZeroizer>` and proves at the source boundary that `SecureZeroize` exists, is `noexcept` and returns `void`.

### `Sha256BlockBytes`

**Classification:** PRIVATE IMPLEMENTATION

The 64-byte SHA-256/HMAC key-block width. It sizes fixed HMAC scratch.

### `Sha256DigestBytes`

**Classification:** PRIVATE IMPLEMENTATION

The 32-byte SHA-256 digest width. A compile-time assertion requires it to equal EDP-Security's `DeploymentWorkingKeyBytes`.

## Private retained state

### `_zeroizer`

**Classification:** PRIVATE IMPLEMENTATION

A non-owning pointer to the Bootstrap-selected zeroizer. It is written only at construction and remains valid for the source lifetime by Composition lifecycle ordering.

### `_applicationKey`

**Classification:** PRIVATE IMPLEMENTATION

A fixed `std::array` containing the live application-wide root copy. It is populated only by successful `Initialize` and erased by `Destroy`/destruction.

### `_deploymentPsk`

**Classification:** PRIVATE IMPLEMENTATION

A fixed `std::array` containing the live operator/deployment PSK copy. It has the same lifecycle as `_applicationKey`.

### `_initialized`

**Classification:** PRIVATE IMPLEMENTATION

Authoritative lifecycle state. `true` means both root arrays contain live provisioned material. It gates derivation and prevents silent reprovisioning over live roots.

## Private memory helpers

### `Zeroize(std::uint8_t (&)[TBytes])`

**Classification:** PRIVATE IMPLEMENTATION

Forwards a fixed C array to the selected zeroizer. The template extent prevents accidental partial-range erasure at its call sites.

### `Zeroize(std::array<std::uint8_t, TBytes>&)`

**Classification:** PRIVATE IMPLEMENTATION

Equivalent fixed-extent erasure for `std::array` root and information storage.

### `CopyBytes`

**Classification:** PRIVATE IMPLEMENTATION

~~~cpp
static void CopyBytes(
    std::uint8_t* destination,
    const std::uint8_t* source,
    std::size_t size
) noexcept;
~~~

Performs a bounded byte copy without introducing a library primitive which might conceal allocation or add another provider dependency. Call sites validate sizes and pointers before invoking it.

## Private cryptographic operations

### `Sha256`

**Classification:** PRIVATE IMPLEMENTATION

Hashes a bounded list of borrowed byte segments through `mbedtls_sha256_starts`, `mbedtls_sha256_update` and `mbedtls_sha256_finish`.

It allocates no heap memory in provider code, skips empty segments, rejects invalid segments, always frees the Mbed TLS context, and erases the digest on failure.

### `HmacSha256`

**Classification:** PRIVATE IMPLEMENTATION

Computes HMAC-SHA-256 with a 64-byte fixed key block and 32-byte inner digest.

Keys longer than one SHA-256 block are normalized by hashing; shorter keys are copied into the zero-filled block. The same block is transformed from inner to outer pad in place, reducing transient storage.

Every key block and inner digest is erased before return. Failure erases the requested 32-byte output.

### `DeriveHkdfSha256`

**Classification:** PRIVATE IMPLEMENTATION

Performs the exact two-stage Mesh V1 operation:

1. Extract: HMAC with application key as salt and deployment PSK as message;
2. Expand: HMAC with the 32-byte PRK over canonical information followed by the single block index `0x01`.

The derived key remains in unpublished local storage until both stages succeed. PRK and local output are erased on every path.

## Public deterministic-resource constants

### `ApplicationKeyBytes`

**Classification:** PUBLIC API

Exact retained application-key bytes; equal to `TApplicationKeyBytes`.

### `DeploymentPskBytes`

**Classification:** PUBLIC API

Exact retained deployment-PSK bytes; equal to `TDeploymentPskBytes`.

### `ApplicationContextCapacity`

**Classification:** PUBLIC API

Maximum accepted non-secret application context; equal to `TApplicationContextCapacity`.

### `OwnedRootSecretBytes`

**Classification:** PUBLIC API

The sum of the two fixed root extents. This is the exact secret byte payload owned by an initialized instance, excluding pointer/flag/ABI padding.

### `MaximumDerivationInfoBytes`

**Classification:** PUBLIC API

The exact maximum contiguous canonical information buffer:
`DeploymentKeyDerivationInfoFixedBytes + ApplicationContextCapacity`.

## Construction and special members

### Constructor

~~~cpp
explicit ProvisionedDeploymentKeySourceProvider(
    TZeroizer& zeroizer
) noexcept;
~~~

Borrows a lifetime-stable selected zeroizer and creates an unprovisioned source with zero-initialized root arrays.

### Destructor

Calls `Destroy`. Live roots are therefore erased even when application lifecycle code omits explicit shutdown.

### Deleted copy/move operations

The copy constructor, copy assignment, move constructor and move assignment are all deleted. Root ownership and the borrowed dependency binding cannot be duplicated or transferred implicitly.

## Public lifecycle operations

### `Initialize`

~~~cpp
ProvisionedDeploymentKeySourceInitializationResult Initialize(
    ESPressio::Security::ByteView applicationKey,
    ESPressio::Security::ByteView deploymentPsk
) noexcept;
~~~

Validates exact extents, copies both roots, then publishes `_initialized=true`.

The caller continues to own both input buffers and must erase its staging copies. No pointer into caller storage is retained.

An already initialized call preserves existing roots and reports `AlreadyInitialized`.

### `Destroy`

~~~cpp
void Destroy() noexcept;
~~~

When initialized, erases deployment PSK then application key and clears the lifecycle flag. Calling it while uninitialized is an idempotent no-op.

After destruction the same object may be provisioned again.

### `Derive`

~~~cpp
ESPressio::Security::DeploymentKeyDerivationResult Derive(
    const ESPressio::Security::DeploymentKeyDerivationRequest& request,
    ESPressio::Security::MutableByteView output
) noexcept;
~~~

Validates:

- the complete generic derivation request;
- application context not exceeding the template capacity;
- a structurally valid output of exactly 32 bytes;
- availability of both roots.

It writes the canonical transcript with `WriteDeploymentKeyDerivationInfo`, performs HKDF and publishes the output only after success.

Results:

- `Succeeded` — exact working key published;
- `InvalidRequest` — generic request invalid or context too large;
- `InvalidOutput` — output invalid or not exactly 32 bytes;
- `RootSecretUnavailable` — source not initialized;
- `ProviderFailure` — canonical encoding invariant or Mbed TLS SHA processing failed.

Invalid request/output/root-unavailable results preserve caller output. `ProviderFailure` clears it.

## Resource and concurrency contract

One instance has compile-time fixed storage. One derivation has compile-time bounded stack storage and no provider-owned heap allocation or input-dependent queueing.

Calls on one instance are caller-serialized. The provider owns no lock, task or worker and does not wait. It is not an ISR surface.

## Framework alias

### `Framework`

**Classification:** INTERNAL PROVIDER CONTRACT

A local alias for `ESPressio::System::CompositionFramework` used only to express offer/requirement metadata.

## Conditional compilation

When the externally supplied `ESP_PLATFORM` macro is defined and `MBEDTLS_CONFIG_FILE` is absent, the header selects `mbedtls/esp_config.h` before including SHA-256 declarations.

The provider requires Mbed TLS SHA-256, but does not require `CONFIG_MBEDTLS_HKDF_C`.
