# Native strongly ordered atomic words

This example introduces the native ESP32/ESP32-S3 AtomicWord32 provider and tests ordering across two cores. Each Word occupies four bytes and must remain in qualified internal DRAM. Use the optional strong operations when a protocol requires one sequentially consistent order across different words.

1. Select the existing native provider through its narrow header:

```cpp
#include <ESPressio_Platform_ESP_IDF_AtomicWord32.hpp>

using Atomic = ESPressio::Platform::ESPIDF::Concurrency::AtomicWord32Provider;
Atomic::Word state;  // Zero-initialized; static/internal DRAM placement.
```

2. Publish/read the same word with the explicit strong operations:

```cpp
state.StoreSequentiallyConsistent(1U);
const std::uint32_t observed = state.LoadSequentiallyConsistent();
```

These operations do not make an arbitrary lifetime or multi-step application protocol safe by themselves. Retain the word's containing storage and use a reviewed protocol.

3. Open the complete raw source in the selected framework form. Example.hpp is identical in all three directories; no source generation script is required. The files declare two Words, fixed 4096-byte native helper stacks/control blocks, pinned entries, per-round coordination, the forbidden-observation assertion, parked cleanup and the ordinary framework entry.

4. For PlatformIO/PIOArduino, build and upload the selected project using an explicitly selected physical port:

```sh
pio run -d demos/atomic-word-strong/PlatformIO_Arduino -e esp32dev
pio run -d demos/atomic-word-strong/PlatformIO_ESP-IDF -e esp32s3
```

All lib_deps entries are GitHub URLs. Arduino IDE uses Arduino_IDE/Arduino_IDE.ino with the ESP32 board package, the EDP libraries from those same references and C++20 enabled. S3 USB CDC settings must match the selected board/profile. The companion official CLI profile supplies compiler.cpp.extra_flags=-std=gnu++20.

5. Capture the actual board at 115200 baud. Require the complete NATIVE_STRONG_ATOMIC_PASS rounds=10000 words_bytes=8 forbidden=0 cores=0,1 line and no failure/panic marker. Compilation alone is not runtime acceptance. The two-core rounds and observed sizes do not prove worst-case latency or an approved Profiling budget.

See ../../docs/ATOMIC-WORD-32.MD for native placement, unsupported-target refusal and exact validation limits.
