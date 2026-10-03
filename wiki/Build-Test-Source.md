# Build, Test and Source Map

## Baselines

- Language: C++20.
- Target SDK: ESP-IDF/Arduino-ESP32 through PIOArduino.
- Package version: `0.1.0`.
- Source/test checkpoint: `524a881b9ed27a86c5b886429558242d88962b70`.
- Docs/demo/manifest checkpoint: `f7c5667e8359e29d9c2a23f4aa44b8761ca6ac60`.

## Durable documentation

Repository `docs/` covers execution context, spin lock, randomness, AES-GCM, deployment-key source/zeroization and memory resources.

## Host validation

`tests/CMakeLists.txt` builds four strict suites. The deployment-key suite uses OpenSSL only in its host SHA seam, providing an independent digest implementation for fixed known-answer vectors.

Validated on AI-AGENT-02:

- GCC four-suite clean lane: `20261003T212848-f9570e22`;
- Clang four-suite lane: `20261003T212909-2263e37e`;
- GCC ASan/UBSan four-suite lane: `20261003T212918-f70653b0`.

## Target build validation

`demos/deployment-key-source-basic` has all required Arduino IDE, PlatformIO Arduino and PlatformIO ESP-IDF surfaces.

Exact local feature-baseline builds on ESP32-WROOM-32-class `esp32dev`:

- Arduino: `20261003T214228-61597b55` — 22,244 bytes reported RAM, 260,920 bytes application flash;
- ESP-IDF: `20261003T214303-d9da369b` — 12,632 bytes reported RAM, 170,481 bytes application flash.

The local-baseline build copies only generated build inputs into `/tmp` and points include paths at the exact authorized repository checkouts; production source is unchanged.

These are compile/link/size results, not physical flash/runtime evidence. Physical hardware validation remains a separate gate.

## Source navigation

Every production header is listed in [Reference Index](Reference-Index.md). Test mocks under `tests/host/include` are test-only seams and are not production Mbed TLS substitutes.
