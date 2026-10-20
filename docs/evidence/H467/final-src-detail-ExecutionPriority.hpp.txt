#pragma once

#include <freertos/FreeRTOS.h>

#include <ESPressio_Platform.hpp>

namespace ESPressio::Platform::ESPIDF::Execution::Detail {

    /// Maps one portable ESPressio execution-priority level onto the configured native FreeRTOS priority range.
    ///
    /// The mapping is monotonic. Critical uses the highest native priority, Low avoids the
    /// idle priority whenever the native scheduler exposes at least one non-idle level, and
    /// small native ranges may intentionally collapse adjacent ESPressio levels.
    constexpr UBaseType_t NativePriorityFor(
        ESPressio::Platform::Execution::ExecutionPriority priority
    ) noexcept {
        const auto highest = static_cast<UBaseType_t>(
            configMAX_PRIORITIES - 1U
        );

        if (highest == 0U) { return 0U; }

        switch (priority) {
            case ESPressio::Platform::Execution::ExecutionPriority::Low:
                return static_cast<UBaseType_t>(
                    1U <= highest
                        ? 1U
                        : highest
                );

            case ESPressio::Platform::Execution::ExecutionPriority::Normal:
                return static_cast<UBaseType_t>(
                    (highest + 1U) / 2U
                );

            case ESPressio::Platform::Execution::ExecutionPriority::High:
                return static_cast<UBaseType_t>(
                    (highest * 3U + 3U) / 4U
                );

            case ESPressio::Platform::Execution::ExecutionPriority::Critical:
                return highest;
        }

        return static_cast<UBaseType_t>(
            (highest + 1U) / 2U
        );
    }

} // ESPressio::Platform::ESPIDF::Execution::Detail
