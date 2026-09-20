#pragma once

#include <cstdint>

#include <ESPressio_Platform.hpp>

namespace ESPressio::Platform::ESPIDF::Concurrency {

    namespace Framework = ESPressio::System::CompositionFramework;

    /// Compiler-proven target capability for native four-byte atomic operations.
    inline constexpr bool AtomicWord32NativeLockFree =
        __atomic_always_lock_free(
            sizeof(std::uint32_t),
            nullptr
        );


    /// ESP-IDF provider for the native lock-free 32-bit atomic word capability.
    class AtomicWord32Provider final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Provides<
            Framework::Offer<
                ESPressio::Platform::Concurrency::AtomicWord32,
                Framework::PropertyValue<
                    ESPressio::Platform::Concurrency::LockFree,
                    AtomicWord32NativeLockFree
                >,
                Framework::PropertyValue<ESPressio::Platform::Concurrency::AtomicWordStorageBytes, 4U>
            >
        >
    > {

        public:

            /// Indicates whether the compiler proves this target's four-byte atomics are always lock-free.
            static constexpr bool IsNativeLockFree =
                AtomicWord32NativeLockFree;

            /// Exactly four bytes of ESP-IDF-native atomic storage.
            class Word final {

                private:

                    // Atomic storage.

                    /// Native 32-bit value manipulated through compiler atomic operations.
                    std::uint32_t _value;

                public:

                    // Construction and lifetime.

                    constexpr Word() noexcept :
                        _value(0U) {}

                    Word(const Word&) = delete;
                    Word& operator =(const Word&) = delete;
                    Word(Word&&) = delete;
                    Word& operator =(Word&&) = delete;


                    // Atomic loads.

                    std::uint32_t LoadRelaxed() const noexcept {
                        return __atomic_load_n(
                            &_value,
                            __ATOMIC_RELAXED
                        );
                    }

                    std::uint32_t LoadAcquire() const noexcept {
                        return __atomic_load_n(
                            &_value,
                            __ATOMIC_ACQUIRE
                        );
                    }


                    // Atomic stores.

                    void StoreRelaxed(
                        std::uint32_t value
                    ) noexcept {
                        __atomic_store_n(
                            &_value,
                            value,
                            __ATOMIC_RELAXED
                        );
                    }

                    void StoreRelease(
                        std::uint32_t value
                    ) noexcept {
                        __atomic_store_n(
                            &_value,
                            value,
                            __ATOMIC_RELEASE
                        );
                    }


                    // Atomic compare/exchange.

                    bool CompareExchangeAcqRel(
                        std::uint32_t& expected,
                        std::uint32_t desired
                    ) noexcept {
                        return __atomic_compare_exchange_n(
                            &_value,
                            &expected,
                            desired,
                            false,
                            __ATOMIC_ACQ_REL,
                            __ATOMIC_ACQUIRE
                        );
                    }

            };

    };


    static_assert(
        sizeof(AtomicWord32Provider::Word) == sizeof(std::uint32_t),
        "ESP-IDF AtomicWord32Provider must occupy exactly four bytes"
    );

} // ESPressio::Platform::ESPIDF::Concurrency
