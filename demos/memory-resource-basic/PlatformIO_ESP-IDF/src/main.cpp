#include <ESPressio_Platform_ESP_IDF.hpp>

namespace Demo {

    /// Demonstrates ESP-IDF internal, external-only, and external-preferred Memory resources.
    int Run() noexcept {
        ESPressio::Platform::ESPIDF::Memory::InternalMemoryResourceProvider internal;
        ESPressio::Memory::MemoryBlock internalBlock{};

        if (
            internal.Allocate(
                128U,
                16U,
                internalBlock
            ) != ESPressio::Memory::MemoryAllocationResult::Succeeded
        ) {
            return 1;
        }

        if (
            internal.Release(internalBlock) !=
            ESPressio::Memory::MemoryReleaseResult::Released
        ) {
            return 2;
        }

        ESPressio::Platform::ESPIDF::Memory::ExternalMemoryResourceProvider external;
        ESPressio::Memory::MemoryBlock externalBlock{};
        const auto externalResult = external.Allocate(
            128U,
            16U,
            externalBlock
        );

        if (externalResult == ESPressio::Memory::MemoryAllocationResult::Succeeded) {
            if (
                external.Release(externalBlock) !=
                ESPressio::Memory::MemoryReleaseResult::Released
            ) {
                return 3;
            }
        } else if (externalResult != ESPressio::Memory::MemoryAllocationResult::ResourceExhausted) {
            return 4;
        }

        ESPressio::Platform::ESPIDF::Memory::ExternalPreferredMemoryResourceProvider preferred;
        ESPressio::Memory::MemoryBlock preferredBlock{};

        if (
            preferred.Allocate(
                128U,
                16U,
                preferredBlock
            ) != ESPressio::Memory::MemoryAllocationResult::Succeeded
        ) {
            return 5;
        }

        return preferred.Release(preferredBlock) ==
            ESPressio::Memory::MemoryReleaseResult::Released ? 0 : 6;
    }

} // Demo

extern "C" void app_main() {
    static_cast<void>(Demo::Run());
}
