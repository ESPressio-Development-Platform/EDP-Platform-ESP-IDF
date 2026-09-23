# src/randomness/RandomByteSourceProvider.hpp

**Primary classification:** PUBLIC PROVIDER / EXTENSION API

**Source baseline:** `3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5/src/randomness/RandomByteSourceProvider.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `esp_random.h`
- `ESPressio_Platform.hpp`

## Documented declarations

### `RandomByteSourceProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

ESP-IDF hardware-random provider suitable for cryptographic random-byte requests.

```cpp
class RandomByteSourceProvider final : public Framework::Provider<
        ESPressio::Platform::Domain,
```

### `FillRandomBytes`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Fills caller-owned storage using ESP-IDF's system random source.

A zero-byte request succeeds without accessing destination.

- **Parameter `destination`:** Caller-owned writable byte range.
- **Parameter `byteCount`:** Number of random bytes requested.

```cpp
ESPressio::Platform::Randomness::RandomByteGenerationResult FillRandomBytes(
                std::uint8_t* destination,
                std::size_t byteCount
            ) noexcept
```

### `RandomByteSourceContract`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Compile-time validation of the ESP-IDF RandomByteSource provider.

```cpp
using RandomByteSourceContract = ESPressio::Platform::Randomness::Detail::RandomByteSourceProviderTraits<RandomByteSourceProvider>;
```

