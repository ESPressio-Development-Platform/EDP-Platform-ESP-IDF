# ESP-IDF Provider Tests

`ESPIdfProviderTests.cpp` validates ESP-IDF-owned spin-lock behavior, compile-time execution-context properties, the four-level execution-priority mapping and target MemoryResource behavior.

`tests/CMakeLists.txt` builds strict host suites for:

- MemoryResource capability selection, alignment, external-only failure and external-preferred/internal fallback through a deliberately small `esp_heap_caps.h` simulation;
- RandomByteSource result and caller-buffer behavior;
- AES-256-GCM provider metadata, segmented associated data, encrypt/decrypt behavior and authentication-before-publication through a controlled GCM seam;
- SecretZeroization and ProvisionedDeploymentKeySource contracts, lifecycle, six independent HKDF-SHA-256 known-answer vectors, root/context/purpose/generation separation, failure clearing and zeroization evidence.

The deployment-key SHA-256 host seam delegates digest computation to OpenSSL. It is intentionally independent of the production Mbed TLS implementation and does not replace target compilation.

Provider headers instantiate their corresponding EDP-Platform, EDP-Memory or EDP-Security contract traits, so incompatible surfaces fail at the provider definition boundary.

Buildable demos under `./demos` exercise the target-owned paths. In particular, `deployment-key-source-basic` composes the two concrete ESP-IDF security providers with EDP-Security's generic bounded deployment-key lifecycle.

H428: S3 real 2-core hardware CAS 50,000-count and certified ClockCoordinator Root-refusal physical test lives in EDP-Labs/Mesh-Concurrency-Qualification docs/H428-D-CERTIFIED-RECEIVER-GUARD-ATOMIC-STRESS.MD; WROOM ESP-IDF build succeeded. Xtensa target-only provider, no host execution claims.


## Native AtomicWord32 target/storage refusal

Run `python3 tests/host/AtomicWord32StorageContractTests.py` with sibling
EDP-Platform and EDP-System checkouts, or supply --platform and --system.
GCC and Clang must be available. Controlled SDK stubs verify constructor
refusal under NDEBUG and unsupported-target rejection; no Xtensa CAS is
executed on the host. CTest registers this suite and the four existing
portable host provider executables, retaining assertions in release builds.
The dedicated EDP-Labs H429 application supplies actual hardware evidence.


## Native canonical timebase (H467)

`NativeMonotonicTimebaseContractTests.py` instantiates the actual native provider
with explicit SDK MODEL headers, twelve target/configuration variants on each
of GCC/Clang, typed source/debug/query/counter refusals and canonical conversion.
Use --mode normal, release, sanitize or analyze (Clang). CTest requires
EDP_CLOCK_SOURCE_DIR and registers the same runner; EDP_NATIVE_TIMEBASE_CONTRACT_MODE
selects its mode. These seams make no native instruction or physical rate claim.
See [the native contract](../docs/NATIVE-MONOTONIC-TIMEBASE.MD).
