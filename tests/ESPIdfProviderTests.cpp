#include <cassert>
#include <cstdint>

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

        ESPressio::Platform::ESPIDF::Memory::InternalMemoryResourceProvider internalMemory;
        ESPressio::Memory::MemoryBlock internalBlock{};

        assert(
            internalMemory.Allocate(
                64U,
                16U,
                internalBlock
            ) == ESPressio::Memory::MemoryAllocationResult::Succeeded
        );

        assert(
            reinterpret_cast<std::uintptr_t>(internalBlock.Address) %
            internalBlock.Alignment == 0U
        );

        assert(
            internalMemory.Release(internalBlock) ==
            ESPressio::Memory::MemoryReleaseResult::Released
        );

        ESPressio::Platform::ESPIDF::Memory::ExternalMemoryResourceProvider externalMemory;
        ESPressio::Memory::MemoryBlock externalBlock{};
        const auto externalResult = externalMemory.Allocate(
            64U,
            16U,
            externalBlock
        );

        if (externalResult == ESPressio::Memory::MemoryAllocationResult::Succeeded) {
            assert(
                externalMemory.Release(externalBlock) ==
                ESPressio::Memory::MemoryReleaseResult::Released
            );
        } else {
            assert(
                externalResult ==
                ESPressio::Memory::MemoryAllocationResult::ResourceExhausted
            );
        }

        ESPressio::Platform::ESPIDF::Memory::ExternalPreferredMemoryResourceProvider preferredMemory;
        ESPressio::Memory::MemoryBlock preferredBlock{};

        assert(
            preferredMemory.Allocate(
                64U,
                16U,
                preferredBlock
            ) == ESPressio::Memory::MemoryAllocationResult::Succeeded
        );

        assert(
            preferredMemory.Release(preferredBlock) ==
            ESPressio::Memory::MemoryReleaseResult::Released
        );

        return 0;
    }

} // Test
