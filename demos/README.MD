# Demos

The ESP-IDF concrete provider repository includes:

- `spin-lock-basic` — ESP-IDF `portMUX_TYPE` critical-section spin lock;
- `execution-context-basic` — static caller-storage-backed execution context with core affinity;
- `memory-resource-basic` — internal, external/PSRAM-only and external-preferred/internal-fallback MemoryResource providers;
- `random-byte-source-basic` — cryptographically suitable random bytes written directly into caller-owned storage;
- `aes-256-gcm-basic` — direct AES-256-GCM authenticated encryption through the ESP-IDF/Mbed TLS provider;
- `deployment-key-source-basic` — provisioned application/deployment roots, secure caller-buffer erasure, six-key derivation, rotation and retirement through the generic EDP-Security key provider.

Each demo provides Arduino IDE, PlatformIO Arduino and PlatformIO ESP-IDF surfaces as required by ESPressio policy.

The Arduino surfaces target Arduino-ESP32, where the underlying ESP-IDF headers and APIs are available. PlatformIO Arduino disables compatibility filtering locally because this repository is correctly declared as ESP-IDF-owned.

The external-only MemoryResource demonstration treats `ResourceExhausted` as a valid outcome on targets/configurations without available SPI RAM; the preferred resource must still be able to fall back to internal memory when suitable internal storage exists.

The deployment-key source demonstration uses deterministic bytes only to make the lifecycle buildable and inspectable. Production applications must provision high-strength roots through an application-authorized path, erase every caller-owned staging copy and never embed demonstration credentials.

The `spin-lock-basic` variants intentionally include the narrow `<ESPressio_Platform_ESP_IDF_SpinLock.hpp>` entry point, proving that SpinLock consumers need not import the broad ESP-IDF provider umbrella.

## C++ Language Baseline

All PlatformIO-compatible demos require C++20 and explicitly select a maintained PIOArduino ESP32 platform package. PIOArduino is the recommended ESP32 PlatformIO-compatible environment for ESPressio.
