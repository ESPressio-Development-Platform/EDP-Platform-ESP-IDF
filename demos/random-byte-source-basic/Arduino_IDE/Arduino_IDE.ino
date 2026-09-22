#include <Arduino.h>

#include <cstdint>

#include <ESPressio_Platform_ESP_IDF.hpp>

namespace Demo {

    /// Exercises the ESP-IDF cryptographic random-byte provider using caller-owned storage.
    int Run() noexcept {
        ESPressio::Platform::ESPIDF::Randomness::RandomByteSourceProvider random;
        std::uint8_t bytes[16U]{};

        return random.FillRandomBytes(
            bytes,
            sizeof(bytes)
        ) == ESPressio::Platform::Randomness::RandomByteGenerationResult::Succeeded
            ? 0
            : 1;
    }

} // Demo


/// Runs the random-provider demonstration once.
void setup() {
    static_cast<void>(
        Demo::Run()
    );
}

/// Leaves the demonstration idle after the one-time run.
void loop() {}
