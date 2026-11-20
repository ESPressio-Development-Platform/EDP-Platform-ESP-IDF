#include "ModelRecord.hpp"

namespace ESPressio::Platform::ESPIDF::Execution::Tests {

    const Record* ReadFromOtherTranslationUnit() noexcept {
        const Provider provider;
        return provider.ReadCurrent().GetIfBound();
    }

} // ESPressio::Platform::ESPIDF::Execution::Tests
