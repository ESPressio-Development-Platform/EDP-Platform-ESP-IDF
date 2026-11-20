#pragma once

#include <memory>
#include <type_traits>

#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ESPressio_Platform_CurrentContextBinding.hpp>

#if !defined(__XTENSA__) || \
    (!defined(CONFIG_IDF_TARGET_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S3))
#error "Native current-context binding requires qualified ESP32 or ESP32-S3 compiler TLS"
#endif

#include <xtensa/config/core-isa.h>

#if !defined(XCHAL_HAVE_THREADPTR) || !XCHAL_HAVE_THREADPTR
#error "Native current-context binding requires the THREADPTR register"
#endif

#if !defined(__cpp_constinit) || __cpp_constinit < 201907L
#error "Native current-context binding requires C++20 constinit"
#endif

namespace ESPressio::Platform::ESPIDF::Execution {

    /// Borrows one owner record in compiler-native TLS, distinct from FreeRTOS application TLS slots.
    /// Select exactly one provider/record Type in application System Composition. Records remain
    /// address stable through retirement and every outstanding read. This provider allocates no IDs.
    /// Qualification is specific to the documented SDK/toolchain; other ports are unsupported.
    /// @tparam TContextRecord Unqualified owner record with firmware-qualified storage/lifetime.
    template<class TContextRecord>
    class CurrentContextBindingProvider final : public System::CompositionFramework::Provider<
        ESPressio::Platform::Domain,
        System::CompositionFramework::Offers<
            System::CompositionFramework::Offer<
                ESPressio::Platform::Execution::CurrentContextBinding,
                System::CompositionFramework::PropertyValue<
                    ESPressio::Platform::Execution::ConstantTimeLookup,
                    true
                >,
                System::CompositionFramework::PropertyValue<
                    ESPressio::Platform::Execution::AllocationFreeLookup,
                    true
                >,
                System::CompositionFramework::PropertyValue<
                    ESPressio::Platform::Execution::CurrentContextOnly,
                    true
                >,
                System::CompositionFramework::PropertyValue<
                    ESPressio::Platform::Execution::RejectsInterruptContexts,
                    true
                >
            >
        >
    > {

        static_assert(
            std::is_object_v<TContextRecord> &&
            std::is_same_v<TContextRecord, std::remove_cv_t<TContextRecord>>,
            "Native binding records must be unqualified object Types"
        );

        private:

            // One trivial native TLS pointer per selected record Type and native task.

            /// Constant initialization forbids a first-lookup initializer or destructor registration.
            inline static constinit thread_local const TContextRecord* _current = nullptr;

            /// Rejects ISR attribution and task-stack TLS access before the scheduler starts.
            /// The qualified SDK scheduler query has bounded cost and takes no kernel lock.
            /// Its SMP variant briefly masks interrupts to read core-local scheduler state.
            static bool IsOrdinaryTaskContext() noexcept {
                if (xPortInIsrContext()) {
                    return false;
                }

                return xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED;
            }

        public:

            // Selected record vocabulary and provider lifetime.

            /// The caller-owned record Type, with no Profiling or Threading dependency.
            using ContextRecord = TContextRecord;

            /// Structured lookup availability and borrowed payload.
            using ReadResult = ESPressio::Platform::Execution::CurrentContextReadResult<ContextRecord>;

            /// Constructs no record, TLS data, registry or native task.
            CurrentContextBindingProvider() noexcept = default;

            /// Keeps a single explicitly supplied provider identity.
            CurrentContextBindingProvider(const CurrentContextBindingProvider&) = delete;

            /// Prevents replacement of the selected provider identity.
            CurrentContextBindingProvider& operator =(const CurrentContextBindingProvider&) = delete;

            /// Prevents moving the selected provider identity.
            CurrentContextBindingProvider(CurrentContextBindingProvider&&) = delete;

            /// Prevents move assignment of the selected provider identity.
            CurrentContextBindingProvider& operator =(CurrentContextBindingProvider&&) = delete;


            // Bounded current-context operations.

            /// Binds only an unoccupied ordinary context; failures preserve the current binding.
            ESPressio::Platform::Execution::CurrentContextBindResult BindCurrent(
                const ContextRecord& record
            ) noexcept {
                using Result = ESPressio::Platform::Execution::CurrentContextBindResult;

                if (!IsOrdinaryTaskContext()) {
                    return Result::UnsupportedContext;
                }

                if (_current != nullptr) {
                    return Result::AlreadyBound;
                }

                _current = std::addressof(record);
                return Result::Bound;
            }

            /// Prevents binding an immediately expiring temporary record.
            ESPressio::Platform::Execution::CurrentContextBindResult BindCurrent(ContextRecord&&) = delete;

            /// Prevents binding an immediately expiring const temporary.
            ESPressio::Platform::Execution::CurrentContextBindResult BindCurrent(const ContextRecord&&) = delete;

            /// Reads only the calling task's record; ISR and pre-scheduler reads carry no payload.
            ReadResult ReadCurrent() const noexcept {
                if (!IsOrdinaryTaskContext()) {
                    return ReadResult::ForUnsupportedContext();
                }

                if (_current == nullptr) {
                    return ReadResult::ForUnbound();
                }

                return ReadResult::ForBound(*_current);
            }

            /// Clears only the exact expected record; failures preserve an existing binding.
            ESPressio::Platform::Execution::CurrentContextRetireResult RetireCurrent(
                const ContextRecord& expectedRecord
            ) noexcept {
                using Result = ESPressio::Platform::Execution::CurrentContextRetireResult;

                if (!IsOrdinaryTaskContext()) {
                    return Result::UnsupportedContext;
                }

                if (_current == nullptr) {
                    return Result::NotBound;
                }

                if (_current != std::addressof(expectedRecord)) {
                    return Result::DifferentBinding;
                }

                _current = nullptr;
                return Result::Retired;
            }

    };

} // ESPressio::Platform::ESPIDF::Execution
