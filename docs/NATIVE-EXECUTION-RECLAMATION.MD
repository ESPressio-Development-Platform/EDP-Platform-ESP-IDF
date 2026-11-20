# Native execution storage reclamation

The completion semaphore means the user entry returned. It is signalled before the native trampoline self-suspends, so it cannot alone prove native nonexecution. Deleting a still-running task on another core can defer cleanup to that core's idle task; immediately reusing caller-owned TCB/stack storage can corrupt the termination list.

Join now observes completion once, retains that fact across timed-out retries, and waits for the native eSuspended state within the SAME requested wait budget. NoWait never delays. The installed ESP-IDF 5.5.5 eTaskGetState acquires the kernel lock and its taskIS_CURRENTLY_RUNNING macro checks both pxCurrentTCBs before reporting a list-based parked state. The provider owns the handle/start gate and never resumes a completed trampoline, so suspended nonexecution is stable until Destroy.

Destroy rechecks parking before deleting. For an unstarted native task still reaching its private start gate, it returns ProviderFailure without releasing storage; the sequential owner may retry. A parked, unstarted task blocked on that private gate is safe to delete. Never release storage following a refused destruction. External SDK deletion/resumption outside the provider's ownership is not supported.

Private WaitBudget::WaitUntil uses NativeReadinessWaitResult (Ready/TimedOut), one inspection for NoWait and one shared tick budget for finite semaphore plus park waits; it does not reset the timeout between phases. The existing Semaphore Take implementation is unchanged.

The actual-provider SDK MODEL tests cover completion-before-park, no-wait retry preserving a consumed completion, finite total budget including tick wrap, unstarted nonexecution and fresh incarnation. The target fixture additionally runs 100 actual provider-created incarnations on alternating cores with the same static TCB/stack. Physical success must be recorded separately.

Public entry: ESPressio_Platform_ESP_IDF_ExecutionContext.hpp. Existing execution operations and result Types are preserved. Actual Threading lifecycle binding integration is separate.
