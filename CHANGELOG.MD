# Changelog

## 0.1.0 — unreleased, mesh_v1

- H429 / #9: restrict the native AtomicWord32 provider to actual ESP32/S3
  S32C1I and require the complete Word range in internal DRAM. Reject
  ESP32-S2 and external/RTC storage instead of silently using SDK emulation.
- Add GCC/Clang target/storage refusal tests, CTest registration, maintained
  GitHub Actions coverage and real WROOM32/S3 two-core qualification.
- Preserve the existing four-byte ABI, operation API, explicit include and
  library version. No Clock authority or NVS/security provisioning change.

- H433 (0.1.0): fix unqualified native cryptographic random success; explicit exclusive SDK source acquisition/release, default/unsupported refusal, noncopyable owner, host and actual C+D two-core qualification. Historical key entropy is not retroactively qualified.

## Unreleased — 0.1.0 current-context binding

- Add optional typed binding vocabulary/native realization boundary.
- Add exact result/ownership tests and three-surface source-led demos.
- Keep native qualification explicit; no release claim.


### Native execution lifetime correction

- Join now proves native self-suspension after entry completion within one total timeout, preserving completion across timed-out retries.
- Destroy refuses live/unparked storage reclamation, preventing cross-core deferred-cleanup/static-TCB reuse races.
- Add actual-source model regressions, native/Arduino physical migration and 100-incarnation owner tests, a narrow execution header and explicit qualification limits.
- Select reintegrated Security main and consistent GitHub URL demo dependencies; add S3 execution-demo build environments.
