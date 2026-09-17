#include <cstdint>

#include <ESPressio_Platform_ESP_IDF.hpp>

namespace Demo {

    /// Runs the AtomicWord32 provider demonstration.
    int Run() noexcept {
        using Provider = ESPressio::Platform::ESPIDF::Concurrency::AtomicWord32Provider;

        Provider::Word word;
        word.StoreRelease(7U);

        std::uint32_t expected = 7U;
        const auto replaced = word.CompareExchangeAcqRel(
            expected,
            9U
        );

        return
            replaced &&
            word.LoadAcquire() == 9U
                ? 0
                : 1;
    }

} // Demo

/// Runs the demonstration from the ESP-IDF application entry point.
extern "C" void app_main() {
    static_cast<void>(Demo::Run());
}
