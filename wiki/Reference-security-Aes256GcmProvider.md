# src/security/Aes256GcmProvider.hpp

**Primary classification:** PUBLIC PROVIDER / EXTENSION API

**Source baseline:** `3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5/src/security/Aes256GcmProvider.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `mbedtls/gcm.h`
- `ESPressio_Memory.hpp`
- `ESPressio_Security.hpp`

## Documented declarations

### `Aes256GcmProcessingResult`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Internal outcome while driving one Mbed TLS GCM streaming operation.

```cpp
enum class Aes256GcmProcessingResult : std::uint8_t
```

### `TByteOperationsProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

ESP-IDF/Mbed TLS AES-256-GCM provider for the EDP-Security AuthenticatedCrypto capability.

- **Template parameter `TByteOperationsProvider`:** Concrete EDP-Memory ByteOperations provider used for bounded
        tail copies without introducing direct C-library memory calls.

```cpp
template<class TByteOperationsProvider>
    class Aes256GcmProvider final : public Framework::Provider<
        ESPressio::Security::Domain,
```

### `_bytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Canonical byte-operation provider selected by Bootstrap.

```cpp
const TByteOperationsProvider& _bytes;
```

### `KeyBytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

AES-256 key bytes.

```cpp
static constexpr std::size_t KeyBytes = 32U;
```

### `TagBytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

AES-GCM authentication tag bytes.

```cpp
static constexpr std::size_t TagBytes = 16U;
```

### `BlockBytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Native AES block bytes used for deterministic streaming chunks.

```cpp
static constexpr std::size_t BlockBytes = 16U;
```

### `TailScratchBytes`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Tail scratch bytes sufficient for one partial GCM update plus finish output.

```cpp
static constexpr std::size_t TailScratchBytes = 32U;
```

### `IsTagEqual`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether two fixed-size tags are equal without data-dependent early exit.

```cpp
static bool IsTagEqual(
                const std::uint8_t* left,
                const std::uint8_t* right
            ) noexcept
```

### `TKeyProvider`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Sets the AES-256 key, starts GCM and feeds every associated-data segment in order.

```cpp
template<class TKeyProvider>
            Detail::Aes256GcmProcessingResult StartOperation(
                mbedtls_gcm_context& context,
                int mode,
                const TKeyProvider& keys,
                const ESPressio::Security::KeySelection& selection,
                ESPressio::Security::ByteView nonce,
                ESPressio::Security::AssociatedDataView associatedData
            ) const noexcept
```

### `ProcessInput`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Processes all complete AES blocks directly and buffers only one final partial block.

```cpp
Detail::Aes256GcmProcessingResult ProcessInput(
                mbedtls_gcm_context& context,
                ESPressio::Security::ByteView input,
                ESPressio::Security::MutableByteView output,
                std::size_t& producedBytes,
                std::uint8_t* tail,
                std::size_t& tailBytes
            ) const noexcept
```

### `TKeyProvider`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Runs one complete GCM pass, optionally publishing plaintext/ciphertext into caller output.

```cpp
template<class TKeyProvider>
            Detail::Aes256GcmProcessingResult RunPass(
                int mode,
                const TKeyProvider& keys,
                const ESPressio::Security::KeySelection& selection,
                ESPressio::Security::ByteView nonce,
                ESPressio::Security::AssociatedDataView associatedData,
                ESPressio::Security::ByteView input,
                ESPressio::Security::MutableByteView output,
                std::uint8_t* calculatedTag
            ) const noexcept
```

### `TKeyProvider`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Calculates the GCM tag without publishing candidate plaintext to caller storage.

```cpp
template<class TKeyProvider>
            Detail::Aes256GcmProcessingResult CalculateDecryptTag(
                const TKeyProvider& keys,
                const ESPressio::Security::KeySelection& selection,
                ESPressio::Security::ByteView nonce,
                ESPressio::Security::AssociatedDataView associatedData,
                ESPressio::Security::ByteView ciphertext,
                std::uint8_t* calculatedTag
            ) const noexcept
```

### `Aes256GcmProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Binds the EDP-Memory byte-operation provider selected by Bootstrap.

```cpp
explicit Aes256GcmProvider(
                const TByteOperationsProvider& bytes
            ) noexcept :
                _bytes(bytes) {}
```

### `TKeyProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Encrypts one caller-owned plaintext range and emits a 16-byte GCM tag.

```cpp
template<class TKeyProvider>
            ESPressio::Security::AuthenticatedEncryptionResult Encrypt(
                const TKeyProvider& keys,
                const ESPressio::Security::KeySelection& selection,
                ESPressio::Security::ByteView nonce,
                ESPressio::Security::AssociatedDataView associatedData,
                ESPressio::Security::ByteView plaintext,
                ESPressio::Security::MutableByteView ciphertext,
                ESPressio::Security::MutableByteView tag
            ) noexcept
```

### `TKeyProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Authenticates ciphertext before publishing plaintext into caller storage.

```cpp
template<class TKeyProvider>
            ESPressio::Security::AuthenticatedDecryptionResult Decrypt(
                const TKeyProvider& keys,
                const ESPressio::Security::KeySelection& selection,
                ESPressio::Security::ByteView nonce,
                ESPressio::Security::AssociatedDataView associatedData,
                ESPressio::Security::ByteView ciphertext,
                ESPressio::Security::ByteView tag,
                ESPressio::Security::MutableByteView plaintext
            ) noexcept
```

## Preprocessor controls in this header

- `#if defined(ESP_PLATFORM) && !defined(MBEDTLS_CONFIG_FILE)`
- `#define MBEDTLS_CONFIG_FILE "mbedtls/esp_config.h"`
