#include <cassert>
#include <cstdint>

#include <ESPressio_Platform_ESP_IDF.hpp>

namespace Memory = ESPressio::Memory;
namespace ESPIDFMemory = ESPressio::Platform::ESPIDF::Memory;

int main() {
    ESPIDFMemory::InternalMemoryResourceProvider internal;
    Memory::MemoryBlock internalBlock{};

    assert(
        internal.Allocate(
            96U,
            32U,
            internalBlock
        ) == Memory::MemoryAllocationResult::Succeeded
    );
    assert((EspIdfTestLastCapabilities & MALLOC_CAP_INTERNAL) != 0U);
    assert(
        reinterpret_cast<std::uintptr_t>(internalBlock.Address) %
        internalBlock.Alignment == 0U
    );
    assert(internal.Release(internalBlock) == Memory::MemoryReleaseResult::Released);

    ESPIDFMemory::ExternalMemoryResourceProvider external;
    Memory::MemoryBlock externalBlock{};
    EspIdfTestFailExternal = true;

    assert(
        external.Allocate(
            64U,
            16U,
            externalBlock
        ) == Memory::MemoryAllocationResult::ResourceExhausted
    );

    ESPIDFMemory::ExternalPreferredMemoryResourceProvider preferred;
    Memory::MemoryBlock preferredBlock{};
    const auto attemptsBefore = EspIdfTestAllocationAttempts;

    assert(
        preferred.Allocate(
            64U,
            16U,
            preferredBlock
        ) == Memory::MemoryAllocationResult::Succeeded
    );
    assert(EspIdfTestAllocationAttempts == attemptsBefore + 2U);
    assert((EspIdfTestLastCapabilities & MALLOC_CAP_INTERNAL) != 0U);
    assert(preferred.Release(preferredBlock) == Memory::MemoryReleaseResult::Released);

    Memory::MemoryBlock unchanged{
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(1U)),
        5U,
        8U
    };
    assert(
        internal.Allocate(
            0U,
            8U,
            unchanged
        ) == Memory::MemoryAllocationResult::InvalidSize
    );
    assert(unchanged.Size == 5U);

    return 0;
}
