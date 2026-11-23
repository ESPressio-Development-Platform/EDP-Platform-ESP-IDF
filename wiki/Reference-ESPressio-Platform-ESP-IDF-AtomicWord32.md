# Native AtomicWord32 narrow entry point

PUBLIC NARROW ENTRY POINT: src/ESPressio_Platform_ESP_IDF_AtomicWord32.hpp includes concurrency/AtomicWord32Provider.hpp without the broad umbrella. It adds no separate runtime object, provider, state or dependency.

[Every provider/Word operation and constraint](Reference-AtomicWord32-ESP-IDF); [normative contract](../docs/ATOMIC-WORD-32.MD); [complete raw-source example](../demos/atomic-word-strong/README.MD).
