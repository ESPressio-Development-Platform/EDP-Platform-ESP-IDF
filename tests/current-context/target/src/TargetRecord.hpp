#pragma once

#include <cstdint>
#include <ESPressio_Platform_ESP_IDF_CurrentContextBinding.hpp>

namespace NativeBindingQualification {

    /// Fixed target fixture record; identities are test inputs, never an allocator.
    struct Record final {

        /// Fresh nonzero test incarnation.
        std::uint64_t identity;

    };

    /// One selected record/provider Type in every target translation unit.
    using Binding = ESPressio::Platform::ESPIDF::Execution::CurrentContextBindingProvider<Record>;

    /// Reads the selected provider from a separately compiled target translation unit.
    const Record* ReadOtherTranslationUnit() noexcept;

    /// Runs the bounded native target qualification sequence.
    void Run();

} // NativeBindingQualification
