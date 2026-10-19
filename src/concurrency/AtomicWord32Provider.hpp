#pragma once
// ESP-IDF H428: Xtensa S32C1I-based hardware CAS, fixed 4-byte atomic
// provider for coherent ClockCoordinator snapshots. No heap or mutex.
// Other architectures must provide independently qualified implementations.

#include <cstdint>
#include <type_traits>
#include <esp_cpu.h>
#include <ESPressio_Platform.hpp>

#if !defined(__XTENSA__) || \
    (!defined(CONFIG_IDF_TARGET_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S2) && \
     !defined(CONFIG_IDF_TARGET_ESP32S3))
#error "ESPIDF AtomicWord32Provider requires a qualified Xtensa ESP32 target"
#endif

namespace ESPressio::Platform::ESPIDF::Concurrency {

class AtomicWord32Provider final : public System::CompositionFramework::Provider<
    ESPressio::Platform::Domain,
    System::CompositionFramework::Offers<
        System::CompositionFramework::Offer<
            ESPressio::Platform::Concurrency::AtomicWord32,
            System::CompositionFramework::PropertyValue<
                ESPressio::Platform::Concurrency::LockFree,true>,
            System::CompositionFramework::PropertyValue<
                ESPressio::Platform::Concurrency::AtomicWordStorageBytes,4U>
        >
    >
> {
public:
    class Word final {
        alignas(4) volatile std::uint32_t _value{0U};

        static inline void MemoryBarrier() noexcept {
            __asm__ __volatile__("memw" ::: "memory");
        }
    public:
        Word() noexcept = default;
        Word(const Word&)=delete;
        Word& operator=(const Word&)=delete;
        Word(Word&&)=delete;
        Word& operator=(Word&&)=delete;

        std::uint32_t LoadRelaxed() const noexcept {return _value;}
        std::uint32_t LoadAcquire() const noexcept {
            const auto v=_value; MemoryBarrier();return v;
        }
        void StoreRelaxed(std::uint32_t v) noexcept {_value=v;}
        void StoreRelease(std::uint32_t v) noexcept {
            MemoryBarrier();_value=v;
        }
        bool CompareExchangeAcqRel(
            std::uint32_t& expected,std::uint32_t desired
        ) noexcept {
            MemoryBarrier();
            const bool success=esp_cpu_compare_and_set(&_value,expected,desired);
            MemoryBarrier();
            if(!success)expected=_value;
            return success;
        }
    };
    static_assert(sizeof(Word)==4U);
    static_assert(alignof(Word)==4U);
    static_assert(!std::is_copy_constructible_v<Word>);
};
} // namespace ESPressio::Platform::ESPIDF::Concurrency
