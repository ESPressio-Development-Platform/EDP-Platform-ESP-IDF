# H363 — Development-lab P-256 keys in ordinary Persistence storage

**NON-HARDENED TEST PROFILE — NOT A PRODUCTION KEYSTORE.**

The user expressly approved proving independent authenticated A/B/C election on
development boards before procuring and irreversibly provisioning dedicated
hardened boards. These P-256 private keys are stored in **ordinary unencrypted
NVS** through the existing EDP-Persistence `KeyValueStorage` interface.
**DO NOT burn eFuses, enable Secure Boot or flash encryption.**

`src/security/DevelopmentP256KeySource.hpp` is an **optional concrete
ESP-IDF/Mbed TLS key-source helper**. A Bootstrap-supplied
`TStorage` provider fulfils the **neutral** `EDP-Persistence`
`KeyValueStorage` contract; `TRandom` fulfils the injected
`EDP-Platform` secure random byte source. The generic `EDP-Mesh`
library must never import this concrete provider.

## Explicit lifecycle

1. Bootstrap selects a physical device identifier, immutable fixed-electorate
   epoch/digest and a nonzero `DigitalSigningKeyReference` identifier/generation.
2. `Recover()` reads a fixed-width 185-byte record, requires an exact version,
   owner identity, epoch, configuration digest and reference generation;
   SHA-256 detects accidental corruption. Re-derive P-256 public key from the
   persisted private scalar; reject invalid scalars and mismatching keys.
3. If and only if `Recover()==Unprovisioned`, an **explicit first-time lab
   provisioning action** `ProvisionOnce()` generates a fresh private scalar
   from injected CSPRNG, derives its public key, commits through storage and
   performs an **independent full record readback** before allowing Sign.
   Existing/mismatched/unknown records are NOT replaced, silently rolled
   over or erased.
4. `ResolveSigningScalar(reference)` remains null until verified readiness,
   and for every stale/nonmatching identifier/generation. Private key material
   is never to be printed, logged, uploaded or committed to repositories.
   Only public keys may be published, e.g. an enrollment manifest.
5. The elected fixed voter public-key map must be configured via Bootstrap
   and frozen before any ElectionSignedGrantVerifier can count an independent
   voter. A previously stored record, or one valid signature by itself, NEVER
   confers Clock Reference authority.

## Verification

H363 genuine Mbed TLS host tests generate **independent B and C** keypairs,
reject wrong-voter public keys, verify real P-256 signing, exercise simulated
storage/reboot recovery, reject wrong owner/epoch/generation, accidental
corruption, false-positive successful storage writes and re-provision.
The existing P-256 host-signature test also runs. Physical nonvoter D
qualification is separate and must be evidenced by its full on-chip
NVS commit/reboot test and guarded complete original ROM backup.

**Explicitly NOT covered:** NVS at-rest confidentiality, malicious flash
tampering, rollback/cloning, debug-port extraction, secure boot, eFuse
antirollback, key destruction by adversarial firmware, power cut mid-commit,
an RF signed-grant emission, certified B+C quorum, canonical Clock Era
change or measured cross-device synchronisation.

No change to neutral EDP-Security or EDP-Mesh package dependencies is needed:
this platform provider imports only the neutral EDP-Persistence contract.
The platform library **does** now declare EDP-Persistence as a package
dependency. Versions remain `0.1.0`.
