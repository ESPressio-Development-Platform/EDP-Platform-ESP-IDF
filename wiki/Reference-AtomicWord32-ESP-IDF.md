# Native AtomicWord32 provider

Source: `src/concurrency/AtomicWord32Provider.hpp`.
Classification: PUBLIC concrete provider satisfying the existing EDP-Platform
capability. Include it explicitly; the umbrella deliberately omits it.

## Composition and ownership

`AtomicWord32Provider` offers EDP-Platform AtomicWord32 with LockFree=true
and AtomicWordStorageBytes=4. It owns no runtime service, heap, task or lock.
Bootstrap owns Word-containing objects and must place every complete Word
in internal DRAM. The provider and EDP-Platform contract consume the existing
System Composition Framework; no higher-domain dependency is introduced.

Only ESP32 and ESP32-S3 with XCHAL_HAVE_S32C1I qualify. SDK emulation on
ESP32-S2 or PSRAM is incompatible. An unsupported target fails compilation.

## Word and private state

`Word` is the noncopyable/nonmovable exact four-byte, four-aligned value.
Its sole private member `_value` is a volatile uint32_t, not an independent
ownership flag or lock. It is deliberately uninitialized until the constructor
validates the full storage range, then set to zero. Every operation requires
a successfully constructed, stationary object.

`RequireInternalStorage()` is a PRIVATE constructor precondition check over
the first and last Word byte through esp_ptr_in_dram. Failure calls abort
even in release builds, before the first value store. External RAM, RTC and
range straddling are rejected. This is not a fallible initialization status
or permission to fall back to a lock. Constructor failure never publishes a Word.

`MemoryBarrier()` is PRIVATE inline Xtensa MEMW with a compiler memory
clobber. It orders surrounding memory without allocating or masking interrupts.

## Operations

- Default constructor: validates storage and creates zero; invalid placement terminates.
- Copy/move constructors and assignments: deleted, preserving storage identity.
- LoadRelaxed: single aligned read with no surrounding ordering.
- LoadAcquire: read, then MEMW before later memory access.
- StoreRelaxed: single aligned store without surrounding ordering.
- StoreRelease: MEMW before publishing the store.
- CompareExchangeAcqRel(expected, desired): MEMW, SDK native CAS and MEMW;
  on failure expected is refreshed. Only validated internal storage reaches
  the SDK, so its external lock path cannot be selected.

Size, alignment and noncopy/nonmove static assertions preserve the owning
AtomicWord32 ABI. Storage qualification retains no extra RAM. No generic ISR
promise is made, and a containing snapshot cannot be relocated into PSRAM.

## Test interpretation

The Python GCC/Clang contract suite uses SDK stubs for refusal/placement and
compile-time target tests, not execution of Xtensa instructions. It fails
against H428. The H429 Labs app uses the real provider and two physical cores:
50,000 increments and 5,000 release/acquire payload handshakes per boot, two
boots each on WROOM32 and S3, no NVS writes or RF. Physical PSRAM construction
was not tested. None of these results establishes a Clock certificate.

See [normative contract](../docs/ATOMIC-WORD-32.MD).
