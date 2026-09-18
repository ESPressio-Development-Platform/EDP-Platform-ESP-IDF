# Demos

The ESP-IDF concrete provider repository includes:

- `spin-lock-basic` — ESP-IDF `portMUX_TYPE` critical-section spin lock;
- `execution-context-basic` — static caller-storage-backed execution context with core affinity.

Each demo provides Arduino IDE, PlatformIO Arduino and PlatformIO ESP-IDF surfaces as required by ESPressio policy.

The Arduino surfaces target Arduino-ESP32, where the underlying ESP-IDF headers are available. PlatformIO Arduino disables compatibility filtering locally because this repository is correctly declared as ESP-IDF-owned.
