# Explicit ESP-IDF cryptographic entropy ownership (H433)

The existing RandomByteSourceProvider now refuses every nonzero request until
Bootstrap explicitly acquires its audited ESP32/S3 internal entropy source.
Construction and FillRandomBytes never enable ADC, I2S or internal RF implicitly.
Zero-byte requests succeed; invalid destinations remain explicit; an unavailable
or unsupported source returns ProviderFailure and leaves caller bytes untouched.
CryptographicallySuitable describes successful byte production under the full
qualified resource lifetime; the feature flag alone never implies readiness.

## Why the correction is necessary

Exact primary ESP-IDF5.5.5 esp_random.h and docs/en/api-reference/system/random.rst
require an active main entropy source for guaranteed true random output. Normal
application startup disables bootloader entropy. esp_fill_random and SDK
getrandom/getentropy share these prerequisites. S3 secondary entropy alone is
not guaranteed by the guide. The old wrapper always succeeded without checking
that condition. H363's default construction did not establish qualification.
This does not prove that any historic private key is predictable or known.
Historical NVS/keys/signatures remain preserved, but their generation entropy
must not be promoted into a new authority claim without qualified reenrollment.

## Bootstrap contract and migration

Instantiate ONE lifetime-stable provider and call AcquireInternalEntropySource
before any nonzero cryptographic consumer request. Bootstrap must reserve ADC,
internal Wi-Fi/Bluetooth RF and, on ESP32, I2S for the ENTIRE held lifetime, and
meet the SDK clock/power prerequisites. Do not initialize or access those
consumers while held. This is an explicit peripheral reservation, not a passive
flag. Join/stop consumers, then ReleaseInternalEntropySource before using those
peripherals. Destruction also releases an actually held source. Acquisition and
release require externally serialized ordinary context, never an ISR. Consumer
lifetime must not overlap release/destruction or bypass the SDK resource owner.

Acquire uses the ALREADY qualified native AtomicWord32 for one process-wide
owner, then calls SDK bootloader_random_enable. Release calls SDK
bootloader_random_disable BEFORE publishing owner availability. A second
instance/TU cannot acquire or borrow the first owner's readiness. Reentry on
the same owner is explicit. Copy/move are deleted so ownership cannot be cloned.
Only audited ESP32/S3 Xtensa hardware paths acquire; other targets refuse.
Default construction now intentionally fails nonzero requests. Applications
must explicitly migrate Bootstrap. No weak fallback or RF-state guess is used.

The three random-byte-source-basic demo forms demonstrate that reservation and
automatic release. They initialize none of the conflicting consumers. Historical
Lab applications are retained as historical source, not silently rewritten to
assert missing entropy provenance. New campaigns use the explicit owner and
fresh independent credentials/session latches; spent H420 keys/nonces remain spent.

## Resources and source ownership

No new package dependency, RNG/crypto algorithm, heap, task or mutex. SDK owns
actual randomness. The existing native AtomicWord32 owns one inline shared
four-byte word, with its constructor enforcing internal DRAM placement. Compiler
startup guard metadata is implementation-dependent; do not infer total global
RAM from the four-byte word alone. Each provider retains one ownership boolean.
All versions remain 0.1.0; existing mesh_v1 only. No irreversible provisioning.

## Validation

20261019T162205-196f4d37 PASS: Release CTest GCC and Clang, five suites each,
including unchanged Memory/AES/key-source/native-atomic contracts and corrected
unqualified/unsupported host RNG refusal. The host SDK stub is NEVER called to
produce apparently qualified bytes. Host tests do not execute Xtensa instructions
or prove positive native entropy. ESP32/S3 positive SDK paths are separately
built and physically tested in H433, with exact source hashes and original logs.

Actual native build 20261019T162748-f953c7fb PASS both ESP-IDF5.5.5 targets.
C physical 20261019T163051-8cf464cf PASS and D
20261019T163053-c45392f6 PASS: two independent reset/UART cycles EACH. Actual
cores0/1 contend while the winner retains ownership until both attempted; exactly
one winner/one busy, successful held SDK fill, cross-TU exclusion, destructor/
release/reacquire and default/nonowner refusal. No random material is logged.
NVS is byte-identical over all20480 bytes per device. Only existing app0x10000
is flashed; no partition/bootloader/erase/key/eFuse/RF/Mesh Clock operation.
This validates resource ownership and documented SDK prerequisite use, not a
statistical min-entropy measurement, old key qualification or Clock authority.

[Full physical report](https://github.com/ESPressio-Development-Platform/EDP-Labs/blob/mesh_concurrency_v1/Mesh-Concurrency-Qualification/docs/H433-QUALIFIED-ENTROPY-PHYSICAL.MD).

20261019T163840-686f07aa PASS exit0: unqualified/unsupported host refusal ASan/UBSan and Clang analyzer. Positive native path remains actual hardware evidence.
