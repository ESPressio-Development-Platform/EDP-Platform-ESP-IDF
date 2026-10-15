# ESP-IDF ECDSA P-256 SHA-256 DigitalSignature Provider (H360)

Source: [`src/security/EcdsaP256Sha256Provider.hpp`](../src/security/EcdsaP256Sha256Provider.hpp).

**Capability:** `EDP-Security::DigitalSignature`, concrete shared
`EcdsaP256Sha256Provider<TKeySource,TRandomSource>` selected by Bootstrap.
`EDP-Mesh` may consume ONLY the neutral EDP-Security provider contract,
not this ESP-IDF class.

`Sign` uses real Mbed TLS SHA-256, ECDSA P-256 with Bootstrap-selected
cryptographic randomness, produces canonical **low-S** fixed 64-byte
`r||s`. `Verify` requires SEC1 P-256 uncompressed 65-byte public key,
checks on-curve, scalar bounds and low-S, SHA-256 hash, authentic ECDSA
verification, and rejects wrong voter keys, changed messages and high-S
malleability. Typed result enums fail closed.

This implementation borrows an **internal Mbed TLS scalar pointer**
synchronously from `TKeySource::ResolveSigningScalar`. A device-unique
secure persistent key store **does not exist yet**, so this provider is a
cryptographic foundation, NOT evidence of independently provisioned durable
voters. Never store or print private scalars in ordinary logs, Git or Wiki.

Testing: `tests/host/EcdsaP256Sha256ProviderHostTests.cpp` uses real
Mbed TLS lib and **test-only** private scalar=1 with public P-256 generator,
never suitable for production keys. Genuine host GCC and Clang tests check
positive sign and negative wrong message/voter, tampering, high-S. CMake
opt-in `EDP_ENABLE_REAL_P256_MBEDTLS_TESTS=ON` requires genuine Mbed TLS
header/library. Cross-compilation `H360EcdsaProviderTargetCompile.cpp`
covers WROOM32 and ESP32-S3 object-only; neither is a linked firmware test.

**No Clock Reference authority:** valid signature alone does not prove
persistent journal readback, monotonic lease/Clock Era, independent 2/3
certificate, RF transmission, key attestation, NVS safe rollback or
submillisecond clocks. These remain mandatory H360 successor tranches.

See [longform implementation contract](../docs/EDP-SIGNATURE-IMPLEMENTATION.MD).
