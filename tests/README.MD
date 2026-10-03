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
