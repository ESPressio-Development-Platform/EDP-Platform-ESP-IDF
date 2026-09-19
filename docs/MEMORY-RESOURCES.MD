# ESP-IDF Memory Resources

## Ownership boundary

`EDP-Platform-ESP-IDF` owns these providers because they use ESP-IDF heap-capability APIs directly. `EDP-Memory` owns only the `MemoryResource` abstraction and generic deterministic topology/Object Pool behavior.

## InternalMemoryResourceProvider

Requests aligned byte-addressable storage with `MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT`. Success therefore requires storage satisfying ESP-IDF internal-memory and ordinary byte-access requirements.

## ExternalMemoryResourceProvider

Requests aligned storage with `MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT`. If suitable external SPI RAM is unavailable or exhausted, allocation returns `ResourceExhausted`; this provider does not fall back.

## ExternalPreferredMemoryResourceProvider

Attempts `MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT` first. If that request cannot be satisfied, it attempts `MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT`.

The fallback is deliberately contained inside one provider because both allocation paths share the ESP-IDF heap-capability ownership model and `heap_caps_free` release path. `MemoryBlock` therefore does not need to retain physical-placement/provider identity.

Applications requiring guaranteed external placement select `ExternalMemoryResourceProvider`, not the preferred/fallback provider.

## Alignment and release

All providers use `heap_caps_aligned_alloc(alignment, size, caps)`. Size must be non-zero and alignment must be a non-zero power of two. The returned `MemoryBlock` records the requested alignment and byte count.

Release validates the block descriptor then calls `heap_caps_free`. No provider constructs C++ objects or owns Object Pool lifecycle.

## Failure semantics

Invalid size/alignment are reported before the ESP-IDF API is invoked. A null allocation result maps to `ResourceExhausted`. The heap-capability allocation functions do not provide a richer recoverable error outcome for this operation.

## Support boundary

External allocation requires the target/configuration to expose suitable SPI RAM. The provider surface may compile on an ESP32 target where external RAM is unavailable; allocation simply cannot succeed from the external-only resource in that circumstance.
