# ESP-IDF Provider Tests

`ESPIdfProviderTests.cpp` validates ESP-IDF-owned spin-lock behavior, compile-time execution-context properties, and target MemoryResource behavior.

The buildable `execution-context-basic` and Memory-resource demos exercise their corresponding runtime surfaces on ESP-IDF/Arduino-ESP32 targets.

`tests/CMakeLists.txt` additionally builds `host/MemoryResourceProviderHostTests.cpp` against a deliberately tiny `esp_heap_caps.h` simulation. That host seam validates capability selection, alignment, external-only failure, and external-preferred/internal-fallback behavior without pretending to validate real ESP32 memory placement.

Provider headers instantiate their corresponding EDP-Platform / EDP-Memory contract traits, so incompatible surfaces fail at the provider definition boundary.
