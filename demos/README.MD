# Demos

The ESP-IDF concrete provider repository includes:

- `spin-lock-basic` — ESP-IDF `portMUX_TYPE` critical-section spin lock;
- `execution-context-basic` — static caller-storage-backed execution context with core affinity;
- `memory-resource-basic` — ESP-IDF internal, external/PSRAM-only, and external-preferred/internal-fallback MemoryResource providers.

Each demo provides Arduino IDE, PlatformIO Arduino and PlatformIO ESP-IDF surfaces as required by ESPressio policy.

The Arduino surfaces target Arduino-ESP32, where the underlying ESP-IDF headers/APIs are available. PlatformIO Arduino disables compatibility filtering locally because this repository is correctly declared as ESP-IDF-owned.

The external-only MemoryResource demonstration treats `ResourceExhausted` as a valid outcome on targets/configurations without available SPI RAM; the preferred resource must still be able to fall back to internal memory when suitable internal storage exists.

## C++ Language Baseline

All PlatformIO-compatible demos require C++20 and explicitly select the maintained PIOArduino ESP32 platform package. PIOArduino is the recommended ESP32 PlatformIO-compatible environment for ESPressio.

- `random-byte-source-basic` — cryptographically suitable random bytes written directly into caller-owned storage through the ESP-IDF provider.
