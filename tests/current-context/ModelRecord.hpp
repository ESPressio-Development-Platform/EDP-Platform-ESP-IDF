#pragma once

#include <cstdint>
#include <ESPressio_Platform_ESP_IDF_CurrentContextBinding.hpp>

namespace ESPressio::Platform::ESPIDF::Execution::Tests {

    /// Host-only record for borrowed pointer identity checks.
    struct Record final {

        /// Test incarnation identity.
        std::uint64_t identity;

    };

    /// Exactly the same selected concrete Type in two translation units.
    using Provider = CurrentContextBindingProvider<Record>;

    /// Reads the actual inline provider's TLS in a separately compiled translation unit.
    const Record* ReadFromOtherTranslationUnit() noexcept;

} // ESPressio::Platform::ESPIDF::Execution::Tests
