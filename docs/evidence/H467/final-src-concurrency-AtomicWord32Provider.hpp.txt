#pragma once

#include <cstdint>
#include <cstdlib>
#include <type_traits>

#include <esp_cpu.h>
#include <esp_memory_utils.h>
#include <ESPressio_Platform.hpp>

#if !defined(__XTENSA__) || \
    (!defined(CONFIG_IDF_TARGET_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S3))
#error "ESPIDF AtomicWord32Provider requires ESP32 or ESP32-S3 native hardware CAS"
#endif

#include <xtensa/config/core-isa.h>

#if !defined(XCHAL_HAVE_S32C1I) || !XCHAL_HAVE_S32C1I
#error "ESPIDF AtomicWord32Provider requires the native S32C1I instruction"
#endif

namespace ESPressio::Platform::ESPIDF::Concurrency {

    /// Hardware-only four-byte atomic provider for ESP32 and ESP32-S3 internal DRAM.
    /// Bootstrap must keep every Word, including containing snapshots, in internal DRAM.
    /// Unsupported storage terminates construction before the first value store; ESP-IDF's
    /// interrupt-masking/external-RAM CAS emulation is never an allowed implementation.
    class AtomicWord32Provider final : public System::CompositionFramework::Provider<
        ESPressio::Platform::Domain,
        System::CompositionFramework::Offers<
            System::CompositionFramework::Offer<
                ESPressio::Platform::Concurrency::AtomicWord32,
                System::CompositionFramework::PropertyValue<
                    ESPressio::Platform::Concurrency::LockFree,
                    true
                >,
                System::CompositionFramework::PropertyValue<
                    ESPressio::Platform::Concurrency::AtomicWordStorageBytes,
                    4U
                >
            >
        >
    > {
    public:
        /// Noncopyable, nonmovable atomic word whose complete storage must be internal DRAM.
        class Word final {
        private:
            // The constructor validates the complete address range before initializing the value.
            /// Sole aligned hardware atomic value, initialized only after storage qualification.
            alignas(4) volatile std::uint32_t _value;

            /// Orders memory operations on either side of an acquire/release boundary.
            static inline void MemoryBarrier() noexcept {
                __asm__ __volatile__("memw" ::: "memory");
            }

            /// Refuses external RAM, RTC memory and any straddling address range without emulation.
            void RequireInternalStorage() const noexcept {
                const auto* first = reinterpret_cast<const std::uint8_t*>(this);
                if (!esp_ptr_in_dram(first) || !esp_ptr_in_dram(first + sizeof(Word) - 1U)) {
                    std::abort();
                }
            }

        public:
            /// Constructs a zero word only in supported internal DRAM; invalid placement is fatal.
            Word() noexcept {
                RequireInternalStorage();
                _value = 0U;
            }

            /// An atomic word cannot acquire a second storage identity by copying.
            Word(const Word&) = delete;

            /// Assignment cannot replace a live atomic word.
            Word& operator=(const Word&) = delete;

            /// Moving would bypass the original storage qualification.
            Word(Word&&) = delete;

            /// Move assignment cannot replace a live atomic word.
            Word& operator=(Word&&) = delete;

            // Canonical AtomicWord32 operations; construction has already qualified immutable storage.

            /// Reads the naturally atomic aligned value without ordering other memory.
            std::uint32_t LoadRelaxed() const noexcept {
                return _value;
            }

            /// Reads the value before publishing an acquire boundary to subsequent memory accesses.
            std::uint32_t LoadAcquire() const noexcept {
                const auto value = _value;
                MemoryBarrier();
                return value;
            }

            /// Stores one naturally atomic aligned value without ordering other memory.
            void StoreRelaxed(std::uint32_t value) noexcept {
                _value = value;
            }

            /// Publishes earlier writes before storing the released value.
            void StoreRelease(std::uint32_t value) noexcept {
                MemoryBarrier();
                _value = value;
            }

            /// Attempts native hardware CAS with acquire/release ordering and refreshed failure evidence.
            bool CompareExchangeAcqRel(
                std::uint32_t& expected,
                std::uint32_t desired
            ) noexcept {
                MemoryBarrier();
                const bool success = esp_cpu_compare_and_set(
                    &_value,
                    expected,
                    desired
                );
                MemoryBarrier();
                if (!success) {
                    expected = _value;
                }
                return success;
            }
        };

        static_assert(sizeof(Word) == 4U);
        static_assert(alignof(Word) == 4U);
        static_assert(!std::is_copy_constructible_v<Word>);
        static_assert(!std::is_move_constructible_v<Word>);
    };

} // ESPressio::Platform::ESPIDF::Concurrency
