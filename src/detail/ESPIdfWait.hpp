#pragma once

#include <cstdint>
#include <limits>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <ESPressio_Platform.hpp>

namespace ESPressio::Platform::ESPIDF::Detail {

    /// Number of nanoseconds in one second.
    inline constexpr std::uint64_t NanosecondsPerSecond = 1000000000ULL;

    /// ESP-IDF FreeRTOS tick resolution rounded upward to whole nanoseconds.
    inline constexpr std::uint64_t WaitResolutionNanoseconds =
        (NanosecondsPerSecond + static_cast<std::uint64_t>(configTICK_RATE_HZ) - 1ULL) /
        static_cast<std::uint64_t>(configTICK_RATE_HZ);


    /// Converts a finite nanosecond duration into a rounded-up 64-bit FreeRTOS tick budget.
    inline std::uint64_t ToTotalTicks(
        ESPressio::Platform::Synchronization::WaitTimeout timeout
    ) noexcept {
        if (!timeout.IsFinite()) { return 0U; }

        const auto nanoseconds = timeout.Nanoseconds();
        const auto wholeSeconds = nanoseconds / NanosecondsPerSecond;
        const auto remainingNanoseconds = nanoseconds % NanosecondsPerSecond;
        const auto tickRate = static_cast<std::uint64_t>(configTICK_RATE_HZ);

        if (
            tickRate != 0U &&
            wholeSeconds > std::numeric_limits<std::uint64_t>::max() / tickRate
        ) {
            return std::numeric_limits<std::uint64_t>::max();
        }

        auto ticks = wholeSeconds * tickRate;

        const auto fractionalTicks = (
            remainingNanoseconds * tickRate + NanosecondsPerSecond - 1ULL
        ) / NanosecondsPerSecond;

        if (
            fractionalTicks >
            std::numeric_limits<std::uint64_t>::max() - ticks
        ) {
            return std::numeric_limits<std::uint64_t>::max();
        }

        ticks += fractionalTicks;

        return ticks == 0U ? 1U : ticks;
    }


    /// Tracks one total ESP-IDF FreeRTOS wait budget across native semaphore acquisitions.
    class WaitBudget final {

        private:

            // Wait budget state.

            /// Original ESPressio wait request.
            ESPressio::Platform::Synchronization::WaitTimeout _timeout;

            /// Remaining finite wait budget expressed in native ticks.
            std::uint64_t _remainingTicks;

            /// Last observed native tick coordinate.
            TickType_t _lastTick;


            // Budget accounting.

            /// Removes native time elapsed since the previous budget observation.
            void ConsumeElapsed() noexcept {
                if (!_timeout.IsFinite() || _remainingTicks == 0U) { return; }

                const auto now = xTaskGetTickCount();
                const auto elapsed = static_cast<TickType_t>(
                    now - _lastTick
                );

                _lastTick = now;

                const auto elapsedTicks = static_cast<std::uint64_t>(elapsed);

                if (elapsedTicks >= _remainingTicks) {
                    _remainingTicks = 0U;
                    return;
                }

                _remainingTicks -= elapsedTicks;
            }

            /// Removes elapsed time after one finite native wait operation.
            void ConsumeWait(
                TickType_t before,
                TickType_t after,
                std::uint64_t requestedChunk,
                bool succeeded
            ) noexcept {
                const auto elapsed = static_cast<TickType_t>(
                    after - before
                );
                const auto elapsedTicks = static_cast<std::uint64_t>(elapsed);

                _lastTick = after;

                if (elapsedTicks >= _remainingTicks) {
                    _remainingTicks = 0U;
                    return;
                }

                if (elapsedTicks > 0U) {
                    _remainingTicks -= elapsedTicks;
                    return;
                }

                if (!succeeded) {
                    _remainingTicks -= requestedChunk;
                }
            }

        public:

            // Construction.

            /// Creates a wait budget from one ESPressio timeout.
            explicit WaitBudget(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept :
                _timeout(timeout),
                _remainingTicks(ToTotalTicks(timeout)),
                _lastTick(xTaskGetTickCount()) {}


            // Native waiting.

            /// Takes one ESP-IDF FreeRTOS semaphore while preserving the complete wait budget.
            bool Take(
                SemaphoreHandle_t handle
            ) noexcept {
                if (handle == nullptr) { return false; }

                if (_timeout.IsForever()) {
                    return xSemaphoreTake(
                        handle,
                        portMAX_DELAY
                    ) == pdTRUE;
                }

                if (_timeout.IsNoWait()) {
                    return xSemaphoreTake(
                        handle,
                        static_cast<TickType_t>(0U)
                    ) == pdTRUE;
                }

                ConsumeElapsed();

                constexpr auto maximumChunk =
                    static_cast<std::uint64_t>(portMAX_DELAY) - 1ULL;

                while (_remainingTicks > 0U) {
                    const auto requestedChunk =
                        _remainingTicks < maximumChunk
                            ? _remainingTicks
                            : maximumChunk;

                    const auto before = xTaskGetTickCount();

                    const auto result = xSemaphoreTake(
                        handle,
                        static_cast<TickType_t>(requestedChunk)
                    );

                    const auto after = xTaskGetTickCount();

                    ConsumeWait(
                        before,
                        after,
                        requestedChunk,
                        result == pdTRUE
                    );

                    if (result == pdTRUE) { return true; }
                }

                return false;
            }

    };

} // ESPressio::Platform::ESPIDF::Detail
