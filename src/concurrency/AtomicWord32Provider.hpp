#pragma once

#include <atomic>
#include <cstdint>

#include <ESPressio_Platform.hpp>

namespace ESPressio::Platform::ESPIDF::Concurrency {

    namespace Framework = ESPressio::System::CompositionFramework;

    /// ESP-IDF concrete provider for the Platform AtomicWord32 capability.
    ///
    /// The provider uses the target C++ toolchain's native 32-bit atomic implementation and
    /// deliberately refuses compilation when that implementation is not always lock-free or does
    /// not occupy exactly four bytes. It never falls back to a mutex or SDK synchronization object.
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

        /// Four-byte lock-free atomic word supplied to Platform consumers.
        class Word final {
        private:

            // Atomic storage.

            /// Native 32-bit atomic value owned by this word.
            std::atomic<std::uint32_t> _value;

        public:

            // Construction and lifetime.

            /// Creates an atomic word initialized to zero.
            constexpr Word() noexcept :
                _value(0U) {}

            /// Prevents copying synchronization storage.
            Word(const Word&) = delete;

            /// Prevents copy assignment of synchronization storage.
            Word& operator =(const Word&) = delete;

            /// Prevents relocating synchronization storage after publication.
            Word(Word&&) = delete;

            /// Prevents move assignment of synchronization storage.
            Word& operator =(Word&&) = delete;


            // Atomic loads.

            /// Reads the current value without introducing cross-word ordering.
            std::uint32_t LoadRelaxed() const noexcept {
                return _value.load(
                    std::memory_order_relaxed
                );
            }

            /// Reads the current value and acquires preceding matching release operations.
            std::uint32_t LoadAcquire() const noexcept {
                return _value.load(
                    std::memory_order_acquire
                );
            }


            // Atomic stores.

            /// Replaces the current value without introducing cross-word ordering.
            void StoreRelaxed(
                std::uint32_t value
            ) noexcept {
                _value.store(
                    value,
                    std::memory_order_relaxed
                );
            }

            /// Replaces the current value and releases preceding writes to acquiring readers.
            void StoreRelease(
                std::uint32_t value
            ) noexcept {
                _value.store(
                    value,
                    std::memory_order_release
                );
            }


            // Atomic compare/exchange.

            /// Replaces the current value when it matches the expected value.
            ///
            /// @param expected Expected current value; updated with the observed value on failure.
            /// @param desired Replacement value written on success.
            /// @return true when the replacement was performed; otherwise false.
            bool CompareExchangeAcqRel(
                std::uint32_t& expected,
                std::uint32_t desired
            ) noexcept {
                return _value.compare_exchange_strong(
                    expected,
                    desired,
                    std::memory_order_acq_rel,
                    std::memory_order_acquire
                );
            }

        };

    };


    static_assert(
        std::atomic<std::uint32_t>::is_always_lock_free,
        "ESP-IDF AtomicWord32Provider requires always-lock-free std::atomic<std::uint32_t>"
    );

    static_assert(
        sizeof(std::atomic<std::uint32_t>) == sizeof(std::uint32_t),
        "ESP-IDF AtomicWord32Provider requires four-byte std::atomic<std::uint32_t> storage"
    );

    static_assert(
        sizeof(AtomicWord32Provider::Word) == sizeof(std::uint32_t),
        "ESP-IDF AtomicWord32Provider::Word must occupy exactly four bytes"
    );

    /// Compile-time validation of the complete EDP-Platform AtomicWord32 provider contract.
    using AtomicWord32Contract = ESPressio::Platform::Concurrency::Detail::AtomicWord32ProviderTraits<AtomicWord32Provider>;

    static_assert(
        AtomicWord32Contract::Properties::template Value<ESPressio::Platform::Concurrency::LockFree>,
        "ESP-IDF AtomicWord32Provider must satisfy the lock-free Platform contract"
    );

} // ESPressio::Platform::ESPIDF::Concurrency
