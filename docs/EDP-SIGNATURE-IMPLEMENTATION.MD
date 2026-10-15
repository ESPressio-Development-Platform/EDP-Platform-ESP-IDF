# H360 ECDSA P-256 SHA-256 platform provider (real cryptography, NOT election)

`src/security/EcdsaP256Sha256Provider.hpp` is an **ESP-IDF/Mbed TLS concrete**
provider of the new neutral `EDP-Security::DigitalSignature` capability. It
must be **selected in application Bootstrap via EDP-System Composition**;
`EDP-Mesh` must never import this platform-specific class directly.

Its `Sign` computes SHA-256 over the caller's supplied canonical,
explicitly-domain-separated message, signs using Mbed TLS genuine
`mbedtls_ecdsa_sign` with an EDP-Platform cryptographically suitable random
source, normalizes to low-S and emits canonical fixed-width 64-byte r||s.
`Verify` independently SHA-256 hashes the identical message, verifies the
65-byte uncompressed on-curve SEC1 public key with Mbed TLS, enforces P-256
scalar bounds and low-S and invokes genuine `mbedtls_ecdsa_verify`. Invalid
keys, signatures, wrong messages and wrong voter public keys fail closed.

Provider template parameters are caller-supplied:
- `TKeySource::ResolveSigningScalar(DigitalSigningKeyReference)` returns
  a **borrowed** `const mbedtls_mpi*` for the lifetime of the *synchronous*
  signing call, or null if the generation is missing/invalid. The secret is
  not returned through the neutral EDP-Security API or logged. This SPI is
  explicitly **NOT YET hardened or backed by secure on-chip NVS**; an
  ordinary software scalar held in RAM is accessible to a sufficiently
  compromised device. Private-key provenance, provisioning, attestation,
  irreversible antirollback and purpose authorization remain future gates.
- `TRandomSource::FillRandomBytes(uint8_t*,size_t)` must satisfy the
  cryptographically suitable `EDP-Platform::RandomByteSource` contract;
  ESP-IDF `RandomByteSourceProvider` supplies it via existing Bootstrap.

Neither this provider nor possession of a cryptographically valid signature
proves durable vote journal readback, Clock Era/lease correctness or a true
2-of-3 independent electorate certificate. The application must never
promote the Clock root merely because `Verify` succeeded.

## Genuine cryptographic testing

`tests/host/EcdsaP256Sha256ProviderHostTests.cpp` uses the **real**
Mbed TLS crypto library, private scalar=1 and standard P-256 public G as
explicit host **test-only** fixtures; Linux `getrandom()` supplies host
cryptographic entropy in lieu of ESP-IDF. It tests positive sign/verify
and negative changed-message, invalid bytes, wrong voter key, missing
private key, malformed encoding and ECDSA **high-S malleability** rejection.
Test keys must never be used for actual voter deployments.

Enable the optional genuine test separately from existing fake-stub tests
using `-DEDP_ENABLE_REAL_P256_MBEDTLS_TESTS=ON`, and pass the real matching
`MBEDTLS_INCLUDE_DIR` and `MBEDCRYPTO_LIBRARY`. Never configure this
test against `tests/host/include/mbedtls` fake fixtures.

Also build object-only `tests/host/H360EcdsaProviderTargetCompile.cpp`
with actual Espressif Xtensa WROOM32 and S3 compiler. This is **not**
a full ESP-IDF firmware build and does not test provisioning or RF.

## Next essential architecture tranche

1. Device-unique private-key generation/storage (on-device, protected,
   with explicit generation, secure boot policy and nonexportable extent)
   and immutable fixed-electorate public key distribution, with coherent
   EDP-Security key-generation lifecycle and NVS/power-loss recovery.
2. Canonical signed voter attestation over frozen epoch/digest, voter
   Device/Incarnation, candidate Device/Incarnation, term/proposal,
   Clock Era, non-wrapping persisted grant sequence, lease/fence proof
   and explicit protocol/policy digest.
3. Exactly one durable journal readback followed by signed (independent
   voter) proof + AES transport protection + bounded EDP-Radio ownership;
   no side-path task, copied heap or second replay budget.
4. Candidate B+C quorum certificate, validated Clock transition,
   interruption/partition/failover and finally physically tested NRF24
   and Raw80211. No claims of election authority until then.
