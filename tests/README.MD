# ESP-IDF Provider Tests

`ESPIdfProviderTests.cpp` validates the ESP-IDF-owned spin-lock behavior and compile-time execution-context properties.

The buildable `execution-context-basic` demo exercises the full static execution lifecycle on an ESP-IDF runtime.

Provider headers instantiate their corresponding `EDP-Platform` contract traits, so incompatible surfaces fail at the provider definition boundary.
