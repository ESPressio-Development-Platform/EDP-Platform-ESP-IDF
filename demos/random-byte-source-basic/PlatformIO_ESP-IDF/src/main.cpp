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


/// Runs the random-provider demonstration.
extern "C" void app_main() {
    static_cast<void>(
        Demo::Run()
    );
}
