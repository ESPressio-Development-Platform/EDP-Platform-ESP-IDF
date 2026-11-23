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

## Optional sequentially consistent protocol words

The same provider now advertises SupportsSequentiallyConsistentLoadStore and validates Platform's optional SequentiallyConsistentAtomicWord32ProviderTraits. LoadSequentiallyConsistent and StoreSequentiallyConsistent use GCC/Clang-compatible __atomic builtins with __ATOMIC_SEQ_CST on the existing aligned four-byte value. They do not relabel acquire/release semantics. All words in a multi-word strong protocol must use the same qualified provider/ordering domain and strong operations for every protocol access.

The narrow ESPressio_Platform_ESP_IDF_AtomicWord32.hpp root discovers this opt-in provider without importing the broad umbrella. Construction/storage restrictions and base operations remain unchanged. No extra retained state, dependency, task, allocator, lock or clock is added.

The complete atomic-word-strong demo provides identical Example.hpp content for Arduino IDE, PlatformIO Arduino and PlatformIO ESP-IDF. It runs 10,000 pinned cross-core store/load pairs and rejects the forbidden zero/zero observation. Fixed native helpers coordinate rounds outside the atomic operations; the test's synchronization and polling are not probe work.

Actual SDK-generated strong load/store bodies must be audited independently for direct instructions/barriers without calls, lock/emulation/interrupt masking or retry loops. Host SDK seams validate the contract and refusal behavior but do not prove target instruction ordering. Runtime stress is supporting evidence, not a replacement for the compiler ordering contract. This extension is not a capture gateway or proof of capture/export/archival quiescence. Versions remain 0.1.0.

### 23 November target qualification

The three complete authored forms pass WROOM/S3 SDK builds and physical execution: PIOArduino Arduino, PIOArduino ESP-IDF and official Arduino CLI1.6.0 / esp32 core4.0.0. Each final physical profile performs 10,000 pinned cross-core store/load rounds with no forbidden zero/zero observation. Actual linked NativeStrongLoad/NativeStrongStore seams call the production operations and are executed; all six ELF bodies audit as entry/return, MEMW and one aligned load or store, without calls, branches, retry, masking or fallback. Exact hashes/raw logs/receipts are recorded in the Policy continuation; no GitHub Actions-green or hard latency guarantee is implied.

The maintained SDK runner copies complete authored forms to its output directory and selects the exact checkout ref through a GitHub URL. The instruction-audit runner retains linked body assembly and ELF hashes. Test transport overrides are appliance infrastructure; authored demo manifests contain only public GitHub dependency references. Physical PSRAM construction remains untested; the unchanged constructor's full-range DRAM guard and SDK-stub refusal suite remain explicit.

## Base-only Platform compatibility correction

The optional strong property and trait are exposed only when the coherently selected Platform headers provide the strong contract. Base-only Platform/main users retain the existing provider and four-byte Word; strong consumers must still select the qualified contract and cannot substitute weak ordering. Operation bodies are unchanged.

The original f33e877/Platform-main Clock SDK failure is preserved. RPI continuation validates the same functional correction prepared as AI02 local401ea669 while that appliance is unavailable; source/work is preserved and must be reconciled without force-pushing. Exact current receipts and publication are recorded in Policy.
