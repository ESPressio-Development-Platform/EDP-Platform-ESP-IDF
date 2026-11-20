#include "TargetRecord.hpp"

namespace NativeBindingQualification {

    const Record* ReadOtherTranslationUnit() noexcept {
        const Binding binding;
        return binding.ReadCurrent().GetIfBound();
    }

} // NativeBindingQualification
