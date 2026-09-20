#pragma once

#include <cstdint>

#include <esp_stdatomic.h>

#include <ESPressio_Platform.hpp>

namespace ESPressio::Platform::ESPIDF::Concurrency {

    namespace Framework = ESPressio::System::CompositionFramework;

#if HAS_ATOMICS_32

    /// ESP-IDF provider for the native lock-free 32-bit atomic word capability.
    class AtomicWord32Provider final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Provides<
            Framework::Offer<
                ESPressio::Platform::Concurrency::AtomicWord32,
                Framework::PropertyValue<ESPressio::Platform::Concurrency::LockFree, true>,
                Framework::PropertyValue<ESPressio::Platform::Concurrency::AtomicWordStorageBytes, 4U>
            >
        >
    > {

        public:

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

    using AtomicWord32Contract =
        ESPressio::Platform::Concurrency::Detail::AtomicWord32ProviderTraits<
            AtomicWord32Provider
        >;

#endif

} // ESPressio::Platform::ESPIDF::Concurrency
