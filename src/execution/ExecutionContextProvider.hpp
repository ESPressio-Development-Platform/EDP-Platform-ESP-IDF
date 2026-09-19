#pragma once

#include <cstdint>
#include <limits>

#include <freertos/FreeRTOS.h>
#include <freertos/idf_additions.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <ESPressio_Platform.hpp>

#include "../detail/ESPIdfWait.hpp"

namespace ESPressio::Platform::ESPIDF::Execution {

    namespace Framework = ESPressio::System::CompositionFramework;

#if defined(INCLUDE_uxTaskGetStackHighWaterMark) && ( INCLUDE_uxTaskGetStackHighWaterMark == 1 )
    /// Indicates that this ESP-IDF configuration exposes stack high-water telemetry.
    inline constexpr bool StackTelemetryAvailable = true;
#else
    /// Indicates that this ESP-IDF configuration does not expose stack high-water telemetry.
    inline constexpr bool StackTelemetryAvailable = false;
#endif


#if ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( INCLUDE_vTaskDelete == 1 ) && ( INCLUDE_vTaskSuspend == 1 ) && ( INCLUDE_xTaskGetCurrentTaskHandle == 1 )

    /// ESP-IDF static execution-context provider with processor-affinity support.
    class ExecutionContextProvider final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Provides<
            Framework::Offer<
                ESPressio::Platform::Execution::ExecutionContext,
                Framework::PropertyValue<ESPressio::Platform::Execution::CallerSuppliedStorage, true>,
                Framework::PropertyValue<ESPressio::Platform::Execution::SupportsPriority, true>,
                Framework::PropertyValue<ESPressio::Platform::Execution::SupportsProcessorAffinity, true>,
                Framework::PropertyValue<ESPressio::Platform::Execution::SupportsStackTelemetry, StackTelemetryAvailable>,
                Framework::PropertyValue<ESPressio::Platform::Execution::ControlStorageBytes, sizeof(StaticTask_t)>,
                Framework::PropertyValue<ESPressio::Platform::Execution::ControlStorageAlignment, alignof(StaticTask_t)>,
                Framework::PropertyValue<ESPressio::Platform::Execution::StackStorageAlignment, alignof(StackType_t)>,
                Framework::PropertyValue<ESPressio::Platform::Execution::StackAllocationGranularityBytes, sizeof(StackType_t)>,
                Framework::PropertyValue<
                    ESPressio::Platform::Execution::JoinWaitResolutionNanoseconds,
                    ESPressio::Platform::ESPIDF::Detail::WaitResolutionNanoseconds
                >
            >
        >
    > {

        private:

            // Execution lifecycle state.

            /// Native ESP-IDF FreeRTOS task handle while initialized.
            TaskHandle_t _handle = nullptr;

            /// User entry function invoked after Start.
            ESPressio::Platform::Execution::ExecutionEntry _entry = nullptr;

            /// User parameter passed to the entry function.
            void* _parameter = nullptr;

            /// Indicates whether the execution context was initialized.
            bool _initialized = false;

            /// Indicates whether Start has been accepted.
            bool _started = false;

            /// Indicates whether the owner has successfully joined the completed execution.
            bool _joined = false;


            // Private lifecycle signals.

            /// Static storage for the start gate.
            StaticSemaphore_t _startStorage{};

            /// Static storage for the completion signal.
            StaticSemaphore_t _completionStorage{};

            /// Start-gate semaphore.
            SemaphoreHandle_t _startSignal;

            /// Completion semaphore.
            SemaphoreHandle_t _completionSignal;


            // Native execution entry.

            /// Bridges the ESP-IDF FreeRTOS task entry into the ESPressio execution function.
            static void NativeEntry(
                void* parameter
            ) noexcept {
                auto* self = static_cast<ExecutionContextProvider*>(parameter);

                if (
                    self == nullptr ||
                    self->_startSignal == nullptr ||
                    self->_completionSignal == nullptr
                ) {
                    vTaskSuspend(
                        nullptr
                    );
                    return;
                }

                if (
                    xSemaphoreTake(
                        self->_startSignal,
                        portMAX_DELAY
                    ) != pdTRUE
                ) {
                    vTaskSuspend(
                        nullptr
                    );
                    return;
                }

                const auto entry = self->_entry;

                if (entry != nullptr) {
                    entry(
                        self->_parameter
                    );
                }

                (void)xSemaphoreGive(
                    self->_completionSignal
                );

                for (;;) {
                    vTaskSuspend(
                        nullptr
                    );
                }
            }


            // Priority mapping.

            /// Maps the portable four-level ESPressio priority contract onto the available native range.
            static UBaseType_t NativePriority(
                ESPressio::Platform::Execution::ExecutionPriority priority
            ) noexcept {
                const auto highest = static_cast<UBaseType_t>(configMAX_PRIORITIES - 1U);

                if (highest == 0U) { return 0U; }

                switch (priority) {
                    case ESPressio::Platform::Execution::ExecutionPriority::Low:
                        return static_cast<UBaseType_t>(1U <= highest ? 1U : highest);
                    case ESPressio::Platform::Execution::ExecutionPriority::Normal:
                        return static_cast<UBaseType_t>((highest + 1U) / 2U);
                    case ESPressio::Platform::Execution::ExecutionPriority::High:
                        return static_cast<UBaseType_t>((highest * 3U + 3U) / 4U);
                    case ESPressio::Platform::Execution::ExecutionPriority::Critical:
                        return highest;
                }

                return static_cast<UBaseType_t>((highest + 1U) / 2U);
            }


            // Storage validation.

            /// Reports whether an address satisfies a required alignment.
            static bool IsAligned(
                const void* address,
                std::size_t alignment
            ) noexcept {
                return address != nullptr &&
                    reinterpret_cast<std::uintptr_t>(address) % alignment == 0U;
            }

        public:

            // Construction and lifetime.

            /// Creates an uninitialized ESP-IDF execution provider.
            ExecutionContextProvider() noexcept :
                _startSignal(
                    xSemaphoreCreateBinaryStatic(
                        &_startStorage
                    )
                ),
                _completionSignal(
                    xSemaphoreCreateBinaryStatic(
                        &_completionStorage
                    )
                ) {}

            /// Prevents copying native execution state.
            ExecutionContextProvider(const ExecutionContextProvider&) = delete;

            /// Prevents copy assignment of native execution state.
            ExecutionContextProvider& operator =(const ExecutionContextProvider&) = delete;

            /// Prevents moving native execution state.
            ExecutionContextProvider(ExecutionContextProvider&&) = delete;

            /// Prevents move assignment of native execution state.
            ExecutionContextProvider& operator =(ExecutionContextProvider&&) = delete;


            // Initialization.

            /// Binds caller-owned storage and creates one start-gated ESP-IDF task.
            ESPressio::Platform::Execution::ExecutionInitializationResult Initialize(
                const ESPressio::Platform::Execution::ExecutionStorage& storage,
                const ESPressio::Platform::Execution::ExecutionConfiguration& configuration,
                ESPressio::Platform::Execution::ExecutionEntry entry,
                void* parameter
            ) noexcept {
                if (_initialized) {
                    return ESPressio::Platform::Execution::ExecutionInitializationResult::AlreadyInitialized;
                }

                if (
                    _startSignal == nullptr ||
                    _completionSignal == nullptr
                ) {
                    return ESPressio::Platform::Execution::ExecutionInitializationResult::ProviderFailure;
                }

                if (entry == nullptr) {
                    return ESPressio::Platform::Execution::ExecutionInitializationResult::InvalidConfiguration;
                }

                if (
                    storage.ControlBytes < sizeof(StaticTask_t) ||
                    storage.StackBytes < sizeof(StackType_t) ||
                    storage.StackBytes % sizeof(StackType_t) != 0U ||
                    storage.StackBytes > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()) ||
                    !IsAligned(
                        storage.ControlAddress,
                        alignof(StaticTask_t)
                    ) ||
                    !IsAligned(
                        storage.StackAddress,
                        alignof(StackType_t)
                    )
                ) {
                    return ESPressio::Platform::Execution::ExecutionInitializationResult::InvalidStorage;
                }

                BaseType_t core = tskNO_AFFINITY;

                if (!configuration.Affinity.IsAny()) {
                    const auto requestedCore = static_cast<BaseType_t>(
                        configuration.Affinity.ProcessorIndex()
                    );

                    if (taskVALID_CORE_ID(requestedCore) != pdTRUE) {
                        return ESPressio::Platform::Execution::ExecutionInitializationResult::UnsupportedAffinity;
                    }

                    core = requestedCore;
                }

                while (
                    xSemaphoreTake(
                        _startSignal,
                        0U
                    ) == pdTRUE
                ) {}

                while (
                    xSemaphoreTake(
                        _completionSignal,
                        0U
                    ) == pdTRUE
                ) {}

                _entry = entry;
                _parameter = parameter;
                _started = false;
                _joined = false;

                const auto name = configuration.Name != nullptr
                    ? configuration.Name
                    : "EDP";

                _handle = xTaskCreateStaticPinnedToCore(
                    &NativeEntry,
                    name,
                    static_cast<std::uint32_t>(storage.StackBytes),
                    this,
                    NativePriority(
                        configuration.Priority
                    ),
                    static_cast<StackType_t*>(storage.StackAddress),
                    static_cast<StaticTask_t*>(storage.ControlAddress),
                    core
                );

                if (_handle == nullptr) {
                    _entry = nullptr;
                    _parameter = nullptr;
                    return ESPressio::Platform::Execution::ExecutionInitializationResult::ProviderFailure;
                }

                _initialized = true;

                return ESPressio::Platform::Execution::ExecutionInitializationResult::Succeeded;
            }


            // Lifecycle.

            /// Releases the private start gate and permits the native task to enter user code.
            ESPressio::Platform::Execution::ExecutionStartResult Start() noexcept {
                if (!_initialized || _started) {
                    return ESPressio::Platform::Execution::ExecutionStartResult::InvalidState;
                }

                _started = true;

                if (
                    xSemaphoreGive(
                        _startSignal
                    ) != pdTRUE
                ) {
                    _started = false;
                    return ESPressio::Platform::Execution::ExecutionStartResult::ProviderFailure;
                }

                return ESPressio::Platform::Execution::ExecutionStartResult::Succeeded;
            }

            /// Waits until the user execution entry has returned and records successful ownership join.
            ESPressio::Platform::Execution::ExecutionJoinResult Join(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept {
                if (!_initialized || !_started) {
                    return ESPressio::Platform::Execution::ExecutionJoinResult::InvalidState;
                }

                if (IsCurrentContext()) {
                    return ESPressio::Platform::Execution::ExecutionJoinResult::SelfJoin;
                }

                if (_joined) {
                    return ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded;
                }

                ESPressio::Platform::ESPIDF::Detail::WaitBudget budget(timeout);

                if (!budget.Take(
                    _completionSignal
                )) {
                    return ESPressio::Platform::Execution::ExecutionJoinResult::TimedOut;
                }

                _joined = true;

                return ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded;
            }

            /// Destroys an unstarted context or a context whose completed execution was joined.
            ESPressio::Platform::Execution::ExecutionDestroyResult Destroy() noexcept {
                if (!_initialized || _handle == nullptr) {
                    return ESPressio::Platform::Execution::ExecutionDestroyResult::InvalidState;
                }

                if (_started && !_joined) {
                    return ESPressio::Platform::Execution::ExecutionDestroyResult::InvalidState;
                }

                vTaskDelete(
                    _handle
                );

                _handle = nullptr;
                _entry = nullptr;
                _parameter = nullptr;
                _joined = false;
                _started = false;
                _initialized = false;

                return ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded;
            }


            // Execution inspection.

            /// Reports whether the caller is currently executing inside this native context.
            bool IsCurrentContext() const noexcept {
                return _handle != nullptr &&
                    xTaskGetCurrentTaskHandle() == _handle;
            }

            /// Returns the minimum free stack bytes when ESP-IDF stack telemetry is enabled.
            ESPressio::Platform::Execution::ExecutionStackTelemetry GetStackTelemetry() const noexcept {
#if defined(INCLUDE_uxTaskGetStackHighWaterMark) && ( INCLUDE_uxTaskGetStackHighWaterMark == 1 )
                if (_handle == nullptr) { return {}; }

                return {
                    true,
                    static_cast<std::uint32_t>(
                        uxTaskGetStackHighWaterMark(
                            _handle
                        )
                    )
                };
#else
                return {};
#endif
            }

            /// Yields the current ESP-IDF FreeRTOS execution context.
            static void Yield() noexcept {
                taskYIELD();
            }

    };


    /// Compile-time validation of the ESP-IDF ExecutionContext provider.
    using ExecutionContextContract = ESPressio::Platform::Execution::Detail::ExecutionContextProviderTraits<ExecutionContextProvider>;

#endif

} // ESPressio::Platform::ESPIDF::Execution
