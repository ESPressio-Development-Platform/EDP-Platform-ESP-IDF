#include <cstdint>

#include <ESPressio_Platform_ESP_IDF.hpp>

int main() {
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
        (word.LoadAcquire() == 9U)
            ? 0
            : 1;
}
