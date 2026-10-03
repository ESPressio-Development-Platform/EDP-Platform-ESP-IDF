# src/security/SecretZeroizationProvider.hpp

**Primary classification:** PUBLIC PROVIDER / EXTENSION API

**Source baseline:** `524a881b9ed27a86c5b886429558242d88962b70`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/524a881b9ed27a86c5b886429558242d88962b70/src/security/SecretZeroizationProvider.hpp)

## Purpose

This header supplies the ESP-IDF concrete provider for EDP-Security's shared `SecretZeroization` capability. It delegates erasure to `mbedtls_platform_zeroize` so compiler optimization cannot lawfully remove stores to secret storage.

The provider owns no storage, lock, task or heap allocation.

## Direct includes

- `cstddef`
- `mbedtls/platform_util.h`
- `ESPressio_Security.hpp`

## Composition contract

`SecretZeroizationProvider` is a provider in `ESPressio::Security::Domain`. It offers exactly the shared `ESPressio::Security::SecretZeroization` capability and has no requirements.

It is intended to be selected once and borrowed by other same-domain providers, including the provisioned deployment-key source and generic deployment-key provider.

## Declarations

### `Framework`

**Classification:** INTERNAL PROVIDER CONTRACT

~~~cpp
namespace Framework = ESPressio::System::CompositionFramework;
~~~

A local namespace alias used to express the provider's offer metadata. It does not introduce a new Composition domain.

### `SecretZeroizationProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

~~~cpp
class SecretZeroizationProvider final : public Framework::Provider<
    ESPressio::Security::Domain,
    Framework::Offers<
        Framework::Offer<ESPressio::Security::SecretZeroization>
    >
>
~~~

A stateless concrete provider. Default construction is sufficient. Its lifetime must enclose the lifetime of every provider which borrows it.

The Type is final because substitution occurs through Composition capability selection, not inheritance.

### `SecretZeroizationProvider::SecureZeroize`

**Classification:** PUBLIC PROVIDER OPERATION

~~~cpp
void SecureZeroize(
    ESPressio::Security::MutableByteView bytes
) noexcept;
~~~

Erases one valid non-empty caller-owned range in place.

- Empty valid views are successful no-ops.
- Structurally invalid views are ignored because the generic capability has no failure result and conforming callers must not supply them.
- The operation does not allocate, wait or retain the view.
- The caller must serialize concurrent mutation of overlapping storage.
- It is not presented as an ISR operation.

### `SecretZeroizationContract`

**Classification:** INTERNAL PROVIDER CONTRACT

~~~cpp
using SecretZeroizationContract =
    ESPressio::Security::Detail::SecretZeroizationTraits<
        SecretZeroizationProvider
    >;
~~~

Instantiates EDP-Security's provider trait at the definition boundary. A signature or offer mismatch therefore fails compilation in this repository rather than later in an application topology.

## Conditional compilation

When the externally supplied `ESP_PLATFORM` macro is defined and `MBEDTLS_CONFIG_FILE` is not already defined, this header selects `mbedtls/esp_config.h` before including Mbed TLS.

Neither macro is an EDP-owned application configuration switch. An integrating environment which already defines `MBEDTLS_CONFIG_FILE` retains authority over that selection.
