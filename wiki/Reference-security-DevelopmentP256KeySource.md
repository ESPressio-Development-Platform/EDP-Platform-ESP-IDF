# H363 reference — DevelopmentP256KeySource (lab only)

**Concrete location:** `src/security/DevelopmentP256KeySource.hpp`, selected
by Bootstrap in `EDP-Platform-ESP-IDF`. Depends only on **neutral**
`EDP-Persistence::KeyValueStorage` contract, EDP-Platform secure
`RandomByteSource`, EDP-Security signature key reference and Mbed TLS.
No neutral EDP-Mesh import of an ESP-IDF implementation is allowed.

The device-bound key record contains version, physical DeviceIdentifier,
electorate configuration epoch and digest, opaque signing-key generation,
P-256 private scalar, SEC1 public key and SHA-256 corruption checksum.
**Plain NVS is NOT protected against physical extraction, tampering or
malicious flash rollback.** No eFuse/secure-boot/flash-encryption operations.

`Recover()` is fail-closed on absent, corrupt, foreign-owner or stale
configuration/generation records. Explicit one-time `ProvisionOnce()`
uses approved hardware randomness and must verify committed readback
before `ResolveSigningScalar()` allows the real ECDSA provider to sign.
Bootstrap must enroll each board's public key in a fixed three-member
electorate. A signature alone is NOT a durable vote, quorum or Clock
Reference certificate.

See [full contract and validation](../docs/DEVELOPMENT-P256-NVS-KEYS.MD)
and Policy F12 H363 handoff for exact full-ESP32 physical results.
