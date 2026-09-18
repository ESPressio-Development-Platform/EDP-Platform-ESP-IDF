#include <cassert>

#include <ESPressio_Platform_ESP_IDF.hpp>

namespace Test {

    /// Exercises ESP-IDF-owned provider behavior that does not require a secondary task.
    int Run() noexcept {
        ESPressio::Platform::ESPIDF::Synchronization::SpinLockProvider spinLock;

        spinLock.Acquire();

        assert(
            spinLock.Release() ==
            ESPressio::Platform::Synchronization::SpinLockReleaseResult::Released
        );

#if ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( INCLUDE_vTaskDelete == 1 ) && ( INCLUDE_vTaskSuspend == 1 ) && ( INCLUDE_xTaskGetCurrentTaskHandle == 1 )
        using ExecutionProvider = ESPressio::Platform::ESPIDF::Execution::ExecutionContextProvider;
        using ExecutionContract = ESPressio::Platform::Execution::Detail::ExecutionContextProviderTraits<ExecutionProvider>;

        static_assert(
            ExecutionContract::Properties::template Value<
                ESPressio::Platform::Execution::CallerSuppliedStorage
            >,
            "ESP-IDF execution provider must use caller-supplied storage"
        );

        static_assert(
            ExecutionContract::Properties::template Value<
                ESPressio::Platform::Execution::SupportsProcessorAffinity
            >,
            "ESP-IDF execution provider must advertise processor affinity"
        );
#endif

        return 0;
    }

} // Test
