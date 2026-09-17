#include <cassert>
#include <cstdint>

#include <ESPressio_Platform_ESP_IDF.hpp>

int main() {
    using Provider = ESPressio::Platform::ESPIDF::Concurrency::AtomicWord32Provider;
    using Word = Provider::Word;

    Word word;
    assert(word.LoadAcquire() == 0U);

    word.StoreRelease(17U);
    assert(word.LoadAcquire() == 17U);

    std::uint32_t expected = 17U;
    assert(word.CompareExchangeAcqRel(
        expected,
        29U
    ));
    assert(word.LoadAcquire() == 29U);

    using State = std::uint64_t;
    ESPressio::Platform::Concurrency::ConcurrentSnapshot<State, Provider> snapshot(11ULL);
    assert(snapshot.Read() == 11ULL);

    snapshot.Publish(42ULL);
    assert(snapshot.Read() == 42ULL);

    return 0;
}
