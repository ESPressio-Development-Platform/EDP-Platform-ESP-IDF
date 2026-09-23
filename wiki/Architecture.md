# Architecture

The repository maps abstract EDP contracts to ESP-IDF APIs. Platform providers include ExecutionContext, SpinLock and RandomByteSource. Memory providers expose internal RAM, PSRAM-only and PSRAM-preferred resources through ESP-IDF heap capabilities. Security provides AES-256-GCM authenticated crypto using the ESP-IDF Mbed TLS integration.

API ownership is explicit: FreeRTOS-generic synchronization remains in EDP-Platform-FreeRTOS, while ESP-IDF-specific task affinity, portMUX, heap capabilities and cryptographic integration live here.
