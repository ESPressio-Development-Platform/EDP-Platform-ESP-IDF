# Native AtomicWord32 on ESP32 and ESP32-S3

The opt-in `concurrency/AtomicWord32Provider.hpp` implements the existing
EDP-Platform AtomicWord32 contract. Bootstrap supplies and owns the containing
objects; no task, allocator, mutex or alternative Clock is introduced.

## Target and storage qualification

Only Xtensa ESP32 and ESP32-S3 with `XCHAL_HAVE_S32C1I == 1` qualify.
ESP32-S2 has no native S32C1I instruction, and its SDK fallback disables
interrupts. It is deliberately rejected, as are RISC-V targets and any
otherwise named target which cannot prove the instruction exists.

Every complete four-byte Word must reside in internal DRAM. Its noexcept
constructor checks both ends using the SDK's `esp_ptr_in_dram` predicate
before the first value store. External RAM, RTC memory and a straddling
range terminate through `std::abort()`, including with NDEBUG defined.
There is no fallback to emulated CAS. A containing ConcurrentSnapshot or
ClockCoordinator must therefore also place its Word members in internal DRAM.
Copy and move are deleted, so normal object operations cannot relocate a
qualified Word. Raw relocation is outside the contract.

The restriction is essential: ESP-IDF 5.5.5
`components/esp_hw_support/cpu.c::esp_cpu_compare_and_set` uses an external
RAM lock and interrupt masking for PSRAM. Its
`components/xtensa/include/xt_utils.h::xt_utils_compare_and_set` also masks
interrupts when S32C1I is absent. H428 incorrectly allowed both cases;
H429 corrects that guarantee under EDP-Platform-ESP-IDF#9.

## Operations and resources

| Operation | Semantics |
|---|---|
| Word construction | Validates storage, then initializes exactly one aligned 32-bit value to zero |
| LoadRelaxed / StoreRelaxed | Naturally atomic aligned internal-DRAM access without a memory-order fence |
| LoadAcquire | Read followed by Xtensa MEMW |
| StoreRelease | Xtensa MEMW followed by the store |
| CompareExchangeAcqRel | MEMW, native SDK CAS, MEMW; failure refreshes expected from current storage |

The Composition offer remains LockFree=true and AtomicWordStorageBytes=4
for this qualified storage contract. Word is exactly four bytes aligned to
four bytes, with no retained region flag, hidden lock, heap or task.
The provider does not add a general ISR-safety guarantee.

The header is explicitly included, never imported by the broad platform
umbrella. Existing non-Xtensa consumers of that umbrella remain available.

## Validation scope

`tests/host/AtomicWord32StorageContractTests.py` uses controlled SDK stubs
to verify target refusal, complete-range qualification, abort on unsupported
storage and default/copy/move/size/alignment contracts under GCC and Clang.
It deliberately uses NDEBUG and does not execute or simulate Xtensa CAS.
The test fails against H428 and passes after correction.

The dedicated EDP-Labs H429 application executes the real provider on
independently identified WROOM32 and S3 boards. Each physical boot performs
50,000 CAS increments plus 5,000 release/acquire plain-payload exchanges
between tasks pinned to cores 0 and 1, with two 4 KiB static stacks.
Both boards passed two independently reset boots. Their 20 KiB NVS regions
were read before and after and were byte-for-byte equal. Physical evidence:
[EDP-Labs H429 report](https://github.com/ESPressio-Development-Platform/EDP-Labs/blob/mesh_concurrency_v1/Mesh-Concurrency-Qualification/docs/H429-NATIVE-ATOMIC-PHYSICAL.MD).

This qualifies internal-DRAM atomic behavior on the two actual targets;
physical construction in PSRAM has not been exercised. It does not prove
Clock accuracy, authenticated voter time, election authority, antirollback,
RF timing or a positive receiver certificate. Versions remain 0.1.0.
