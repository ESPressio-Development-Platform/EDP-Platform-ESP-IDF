# src/memory/MemoryResourceProviders.hpp

**Primary classification:** PUBLIC PROVIDER / EXTENSION API

**Source baseline:** `3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5/src/memory/MemoryResourceProviders.hpp)

## Direct includes

- `cstddef`
- `cstdint`
- `esp_heap_caps.h`
- `ESPressio_Memory.hpp`

## Documented declarations

### `IsPowerOfTwo`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Reports whether a non-zero integer is a power of two.

```cpp
constexpr bool IsPowerOfTwo(
            std::size_t value
        ) noexcept
```

### `FixedCapabilityMemoryResourceProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Shared implementation for one fixed ESP-IDF heap-capability mask.

```cpp
template<std::uint32_t TCapabilities>
        class FixedCapabilityMemoryResourceProvider : public Framework::Provider<
            ESPressio::Memory::Domain,
```

### `Allocate`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Allocates one aligned block from heaps satisfying the configured capability mask.

```cpp
ESPressio::Memory::MemoryAllocationResult Allocate(
                    std::size_t byteCount,
                    std::size_t alignment,
                    ESPressio::Memory::MemoryBlock& block
                ) noexcept
```

### `Release`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Releases one block allocated by the ESP-IDF heap-capability allocator.

```cpp
ESPressio::Memory::MemoryReleaseResult Release(
                    const ESPressio::Memory::MemoryBlock& block
                ) noexcept
```

### `InternalMemoryResourceProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

ESP-IDF MemoryResource restricted to byte-addressable internal memory.

```cpp
using InternalMemoryResourceProvider = Detail::FixedCapabilityMemoryResourceProvider<
        MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
    >;
```

### `ExternalMemoryResourceProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

ESP-IDF MemoryResource restricted to byte-addressable external SPI RAM.

```cpp
using ExternalMemoryResourceProvider = Detail::FixedCapabilityMemoryResourceProvider<
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
    >;
```

### `ExternalPreferredMemoryResourceProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

ESP-IDF MemoryResource preferring external SPI RAM and coherently falling back to internal memory.

```cpp
class ExternalPreferredMemoryResourceProvider final : public Framework::Provider<
        ESPressio::Memory::Domain,
```

### `Allocate`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Attempts external SPI RAM first and internal byte-addressable memory second.

```cpp
ESPressio::Memory::MemoryAllocationResult Allocate(
                std::size_t byteCount,
                std::size_t alignment,
                ESPressio::Memory::MemoryBlock& block
            ) noexcept
```

### `Release`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Releases either external or internal storage through the common ESP-IDF heap owner.

```cpp
ESPressio::Memory::MemoryReleaseResult Release(
                const ESPressio::Memory::MemoryBlock& block
            ) noexcept
```

