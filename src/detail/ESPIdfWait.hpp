#pragma once

#include <cstdint>

#include <freertos/FreeRTOS.h>

#include <ESPressio_Platform.hpp>

namespace ESPressio::Platform::ESPIDF::Detail {

    /// Number of nanoseconds in one second.
    inline constexpr std::uint64_t NanosecondsPerSecond = 1000000000ULL;

    /// ESP-IDF FreeRTOS tick resolution rounded upward to whole nanoseconds.
    inline constexpr std::uint64_t WaitResolutionNanoseconds =
        (NanosecondsPerSecond + static_cast<std::uint64_t>(configTICK_RATE_HZ) - 1ULL) /
        static_cast<std::uint64_t>(configTICK_RATE_HZ);


    /// Converts an ESPressio wait request into ESP-IDF FreeRTOS ticks without early timeout.
    inline TickType_t ToTicks(
        ESPressio::Platform::Synchronization::WaitTimeout timeout
    ) noexcept {
        if (timeout.IsForever()) { return portMAX_DELAY; }

        if (timeout.IsNoWait()) { return static_cast<TickType_t>(0U); }

        const auto nanoseconds = timeout.Nanoseconds();
        const auto wholeSeconds = nanoseconds / NanosecondsPerSecond;
        const auto remainingNanoseconds = nanoseconds % NanosecondsPerSecond;
        const auto tickRate = static_cast<std::uint64_t>(configTICK_RATE_HZ);
        const auto maximumFiniteTicks = static_cast<std::uint64_t>(portMAX_DELAY) - 1ULL;

        if (wholeSeconds > maximumFiniteTicks / tickRate) {
            return static_cast<TickType_t>(maximumFiniteTicks);
        }

        auto ticks = wholeSeconds * tickRate;
        const auto fractionalTicks = (
            remainingNanoseconds * tickRate + NanosecondsPerSecond - 1ULL
        ) / NanosecondsPerSecond;

        if (fractionalTicks > maximumFiniteTicks - ticks) {
            return static_cast<TickType_t>(maximumFiniteTicks);
        }

        ticks += fractionalTicks;

        if (ticks == 0ULL) { ticks = 1ULL; }

        if (ticks > maximumFiniteTicks) { ticks = maximumFiniteTicks; }

        return static_cast<TickType_t>(ticks);
    }

} // ESPressio::Platform::ESPIDF::Detail
