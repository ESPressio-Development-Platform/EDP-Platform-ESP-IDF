#pragma once

#include <cstddef>
#include <cstdint>

#include <esp_heap_caps.h>

#include <ESPressio_Memory.hpp>

namespace ESPressio::Platform::ESPIDF::Memory {

    namespace Framework = ESPressio::System::CompositionFramework;

    namespace Detail {

        /// Reports whether a non-zero integer is a power of two.
        constexpr bool IsPowerOfTwo(
            std::size_t value
        ) noexcept {
            return value != 0U && (value & (value - 1U)) == 0U;
        }


        /// Shared implementation for one fixed ESP-IDF heap-capability mask.
        template<std::uint32_t TCapabilities>
        class FixedCapabilityMemoryResourceProvider : public Framework::Provider<
            ESPressio::Memory::Domain,
            Framework::Provides<
                Framework::Offer<ESPressio::Memory::MemoryResource>
            >
        > {

            public:

                /// Allocates one aligned block from heaps satisfying the configured capability mask.
                ESPressio::Memory::MemoryAllocationResult Allocate(
                    std::size_t byteCount,
                    std::size_t alignment,
                    ESPressio::Memory::MemoryBlock& block
                ) noexcept {
                    if (byteCount == 0U) {
                        return ESPressio::Memory::MemoryAllocationResult::InvalidSize;
                    }

                    if (!IsPowerOfTwo(alignment)) {
                        return ESPressio::Memory::MemoryAllocationResult::InvalidAlignment;
                    }

                    void* allocation = heap_caps_aligned_alloc(
                        alignment,
                        byteCount,
                        TCapabilities
                    );

                    if (allocation == nullptr) {
                        return ESPressio::Memory::MemoryAllocationResult::ResourceExhausted;
                    }

                    ESPressio::Memory::MemoryBlock candidate;
                    candidate.Address = allocation;
                    candidate.Size = byteCount;
                    candidate.Alignment = alignment;
                    block = candidate;

                    return ESPressio::Memory::MemoryAllocationResult::Succeeded;
                }

                /// Releases one block allocated by the ESP-IDF heap-capability allocator.
                ESPressio::Memory::MemoryReleaseResult Release(
                    const ESPressio::Memory::MemoryBlock& block
                ) noexcept {
                    if (
                        block.Address == nullptr ||
                        block.Size == 0U ||
                        !IsPowerOfTwo(block.Alignment)
                    ) {
                        return ESPressio::Memory::MemoryReleaseResult::InvalidBlock;
                    }

                    heap_caps_free(block.Address);
                    return ESPressio::Memory::MemoryReleaseResult::Released;
                }

        };

    } // ESPressio::Platform::ESPIDF::Memory::Detail


    /// ESP-IDF MemoryResource restricted to byte-addressable internal memory.
    using InternalMemoryResourceProvider = Detail::FixedCapabilityMemoryResourceProvider<
        MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
    >;


    /// ESP-IDF MemoryResource restricted to byte-addressable external SPI RAM.
    using ExternalMemoryResourceProvider = Detail::FixedCapabilityMemoryResourceProvider<
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
    >;


    /// ESP-IDF MemoryResource preferring external SPI RAM and coherently falling back to internal memory.
    class ExternalPreferredMemoryResourceProvider final : public Framework::Provider<
        ESPressio::Memory::Domain,
        Framework::Provides<
            Framework::Offer<ESPressio::Memory::MemoryResource>
        >
    > {

        public:

            /// Attempts external SPI RAM first and internal byte-addressable memory second.
            ESPressio::Memory::MemoryAllocationResult Allocate(
                std::size_t byteCount,
                std::size_t alignment,
                ESPressio::Memory::MemoryBlock& block
            ) noexcept {
                if (byteCount == 0U) {
                    return ESPressio::Memory::MemoryAllocationResult::InvalidSize;
                }

                if (!Detail::IsPowerOfTwo(alignment)) {
                    return ESPressio::Memory::MemoryAllocationResult::InvalidAlignment;
                }

                void* allocation = heap_caps_aligned_alloc(
                    alignment,
                    byteCount,
                    MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
                );

                if (allocation == nullptr) {
                    allocation = heap_caps_aligned_alloc(
                        alignment,
                        byteCount,
                        MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
                    );
                }

                if (allocation == nullptr) {
                    return ESPressio::Memory::MemoryAllocationResult::ResourceExhausted;
                }

                ESPressio::Memory::MemoryBlock candidate;
                candidate.Address = allocation;
                candidate.Size = byteCount;
                candidate.Alignment = alignment;
                block = candidate;

                return ESPressio::Memory::MemoryAllocationResult::Succeeded;
            }

            /// Releases either external or internal storage through the common ESP-IDF heap owner.
            ESPressio::Memory::MemoryReleaseResult Release(
                const ESPressio::Memory::MemoryBlock& block
            ) noexcept {
                if (
                    block.Address == nullptr ||
                    block.Size == 0U ||
                    !Detail::IsPowerOfTwo(block.Alignment)
                ) {
                    return ESPressio::Memory::MemoryReleaseResult::InvalidBlock;
                }

                heap_caps_free(block.Address);
                return ESPressio::Memory::MemoryReleaseResult::Released;
            }

    };


    static_assert(
        sizeof(ESPressio::Memory::Detail::MemoryResourceProviderTraits<InternalMemoryResourceProvider>) > 0U,
        "ESP-IDF internal MemoryResource provider must satisfy the EDP-Memory contract"
    );

    static_assert(
        sizeof(ESPressio::Memory::Detail::MemoryResourceProviderTraits<ExternalMemoryResourceProvider>) > 0U,
        "ESP-IDF external MemoryResource provider must satisfy the EDP-Memory contract"
    );

    static_assert(
        sizeof(ESPressio::Memory::Detail::MemoryResourceProviderTraits<ExternalPreferredMemoryResourceProvider>) > 0U,
        "ESP-IDF external-preferred MemoryResource provider must satisfy the EDP-Memory contract"
    );

} // ESPressio::Platform::ESPIDF::Memory
