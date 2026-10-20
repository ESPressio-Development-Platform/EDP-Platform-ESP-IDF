#pragma once
#include <cstdio>
#include <timing/EspTimerMonotonicTimebaseProvider.hpp>
#include <clock/MonotonicClockProvider.hpp>
namespace NativeTimebaseExample {
    using Source = ESPressio::Platform::ESPIDF::Timing::EspTimerMonotonicTimebaseProvider;
    using Monotonic = ESPressio::Clock::MonotonicClockProvider<Source>;
    inline void Run() noexcept {
        static Source source;
        static Monotonic canonical(source);
        const auto result = source.InspectSpecificationPrerequisites();
        std::printf("NATIVE_TIMEBASE|prerequisites=%u|source=%u|physical_rate_measured=0|authority=0|now_ns=%llu\n",
            static_cast<unsigned>(result), static_cast<unsigned>(Source::CounterSource()),
            static_cast<unsigned long long>(canonical.Now().Nanoseconds()));
    }
}
