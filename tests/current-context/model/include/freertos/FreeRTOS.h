#pragma once

/// Host SDK MODEL only: no native ISR or scheduler qualification.
inline thread_local bool ModelInIsr = false;
/// Current scheduler availability in this model thread.
inline thread_local int ModelSchedulerState = 1;
/// Model SDK state before task execution.
inline constexpr int taskSCHEDULER_NOT_STARTED = 0;
/// Model SDK ordinary running state.
inline constexpr int taskSCHEDULER_RUNNING = 1;
/// Model SDK suspended scheduler state.
inline constexpr int taskSCHEDULER_SUSPENDED = 2;

/// Models the ordinary/interrupt predicate.
inline bool xPortInIsrContext() noexcept {
    return ModelInIsr;
}
