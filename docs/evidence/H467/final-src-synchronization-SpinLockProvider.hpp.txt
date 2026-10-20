#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>

#include <ESPressio_Platform.hpp>

namespace ESPressio::Platform::ESPIDF::Synchronization {

    namespace Framework = ESPressio::System::CompositionFramework;

    /// ESP-IDF portMUX-backed spin-lock provider.
    ///
    /// Acquisition enters an ESP-IDF critical section and therefore masks interrupts to the
    /// configured critical level while the lock is held. Callers must keep the protected scope short.
    class SpinLockProvider final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Offers<
            Framework::Offer<
                ESPressio::Platform::Synchronization::SpinLock,
                Framework::PropertyValue<
                    ESPressio::Platform::Synchronization::SpinLockSupportsInterruptContext,
                    true
                >
            >
        >
    > {

        private:

            // Native synchronization state.

            /// ESP-IDF SMP-aware critical-section spin lock.
            portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;

        public:

            // Construction and lifetime.

            /// Creates an unlocked ESP-IDF spin lock.
            SpinLockProvider() noexcept = default;

            /// Prevents copying native spin-lock state.
            SpinLockProvider(const SpinLockProvider&) = delete;

            /// Prevents copy assignment of native spin-lock state.
            SpinLockProvider& operator =(const SpinLockProvider&) = delete;

            /// Prevents moving native spin-lock state.
            SpinLockProvider(SpinLockProvider&&) = delete;

            /// Prevents move assignment of native spin-lock state.
            SpinLockProvider& operator =(SpinLockProvider&&) = delete;


            // Ordinary-context lock operations.

            /// Spins until the lock is acquired and enters the ESP-IDF critical section.
            void Acquire() noexcept {
                portENTER_CRITICAL(
                    &_lock
                );
            }

            /// Releases the ordinary-context critical section.
            ESPressio::Platform::Synchronization::SpinLockReleaseResult Release() noexcept {
                portEXIT_CRITICAL(
                    &_lock
                );

                return ESPressio::Platform::Synchronization::SpinLockReleaseResult::Released;
            }


            // Interrupt-context lock operations.

            /// Acquires the spin lock from interrupt context.
            ESPressio::Platform::Synchronization::SpinLockAcquireResult AcquireFromInterrupt() noexcept {
                portENTER_CRITICAL_ISR(
                    &_lock
                );

                return ESPressio::Platform::Synchronization::SpinLockAcquireResult::Acquired;
            }

            /// Releases the spin lock from interrupt context.
            ESPressio::Platform::Synchronization::SpinLockReleaseResult ReleaseFromInterrupt() noexcept {
                portEXIT_CRITICAL_ISR(
                    &_lock
                );

                return ESPressio::Platform::Synchronization::SpinLockReleaseResult::Released;
            }

    };


    /// Compile-time validation of the ESP-IDF SpinLock provider.
    using SpinLockContract = ESPressio::Platform::Synchronization::Detail::SpinLockProviderTraits<SpinLockProvider>;

} // ESPressio::Platform::ESPIDF::Synchronization
