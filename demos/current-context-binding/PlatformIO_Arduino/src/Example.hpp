#pragma once

#include <cstdint>
#include <cstdio>
#include <ESPressio_Platform_ESP_IDF_CurrentContextBinding.hpp>

namespace CurrentContextBindingDemo {

    /// Application-owned, address-stable descriptor.
    struct ContextRecord final {

        /// Nonzero identity chosen by this demo's owner.
        std::uint64_t identity;

        /// Direct slot chosen by this demo's owner.
        std::uint32_t slot;

    };

    /// Explicit concrete selection at the application boundary.
    using Binding = ESPressio::Platform::ESPIDF::Execution::CurrentContextBindingProvider<ContextRecord>;

    /// Bootstrap-selected provider; construction does not bind a task.
    inline Binding binding;

    /// The descriptor remains valid for the complete firmware lifetime.
    inline const ContextRecord applicationContext{1U, 0U};

    /// Runs in the already-started ordinary application task.
    inline void Run() {
        using namespace ESPressio::Platform::Execution;
        const auto bound = binding.BindCurrent(applicationContext);
        if (bound != CurrentContextBindResult::Bound) {
            std::puts("Binding rejected");
            return;
        }

        const auto current = binding.ReadCurrent();
        if (const auto* record = current.GetIfBound()) {
            std::printf(
                "Current context: identity=%llu slot=%lu\n",
                static_cast<unsigned long long>(record->identity),
                static_cast<unsigned long>(record->slot)
            );
        }

        const auto retired = binding.RetireCurrent(applicationContext);
        std::puts(
            retired == CurrentContextRetireResult::Retired
                ? "Binding retired"
                : "Retirement failed"
        );
    }

} // CurrentContextBindingDemo
