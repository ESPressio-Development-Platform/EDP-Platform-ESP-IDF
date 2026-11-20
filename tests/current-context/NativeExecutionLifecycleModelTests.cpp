#include <execution/ExecutionContextProvider.hpp>

#include <cstdio>
#include <cstdlib>

namespace NativeExecutionLifecycleModel {

    using Provider = ESPressio::Platform::ESPIDF::Execution::ExecutionContextProvider;
    using namespace ESPressio::Platform::Execution;
    using ESPressio::Platform::Synchronization::WaitTimeout;

    /// Requires real assertions in both Debug and Release.
    void Require(
        bool value,
        const char* label
    ) {
        if (!value) {
            std::fprintf(
                stderr,
                "LIFECYCLE_MODEL_FAIL %s\n",
                label
            );
            std::abort();
        }
    }

    /// Resets explicit fake SDK state before constructing the actual provider.
    void Reset() {
        ModelTaskState = eReady;
        ModelTick = 0U;
        ModelDelayCalls = 0U;
        ModelDeleteCalls = 0U;
        ModelCreateCalls = 0U;
        ModelSemaphoreCreates = 0U;
        ModelCompletionTakeCost = 0U;
        ModelSuspendAtTick = UINT32_MAX;
        ModelCompletion = nullptr;
        ModelCurrentTask = nullptr;
        ModelDeletedWhileExecuting = false;
    }

    /// Test entry is never executed by the SDK model.
    void Entry(void*) noexcept {}

    /// Owns all storage supplied to the actual execution provider.
    struct Fixture final {

        /// Actual production provider, not a reimplementation.
        Provider provider;

        /// Fake native control backing with real SDK-model alignment.
        StaticTask_t control{};

        /// Fixed caller-owned stack fixture.
        StackType_t stack[2048]{};

        /// Initializes one actual provider against model SDK functions.
        void Initialize() {
            ExecutionStorage storage{};
            storage.ControlAddress = &control;
            storage.ControlBytes = sizeof(control);
            storage.StackAddress = stack;
            storage.StackBytes = sizeof(stack);
            const ExecutionConfiguration configuration{};
            Require(
                provider.Initialize(
                    storage,
                    configuration,
                    Entry,
                    nullptr
                ) == ExecutionInitializationResult::Succeeded,
                "initialize"
            );
        }

        /// Starts the entry gate without fabricating completion.
        void Start() {
            Require(
                provider.Start() == ExecutionStartResult::Succeeded,
                "start"
            );
        }

    };


    /// Completion before parking must not allow successful join or destruction.
    void CheckCompletionBeforeParking() {
        Reset();
        Fixture fixture;
        fixture.Initialize();
        fixture.Start();
        ModelCompletion->count = 1U;
        ModelTaskState = eRunning;

        Require(
            fixture.provider.Join(WaitTimeout::NoWait()) == ExecutionJoinResult::TimedOut,
            "completion before park must time out"
        );
        Require(
            ModelCompletion->count == 0U && ModelDelayCalls == 0U,
            "NoWait consumes completion once without delay"
        );
        Require(
            fixture.provider.Destroy() == ExecutionDestroyResult::InvalidState &&
            ModelDeleteCalls == 0U,
            "no destruction after incomplete native join"
        );

        ModelTaskState = eSuspended;
        Require(
            fixture.provider.Join(WaitTimeout::NoWait()) == ExecutionJoinResult::Succeeded,
            "retry preserves already consumed completion"
        );
        Require(
            fixture.provider.Destroy() == ExecutionDestroyResult::Succeeded &&
            !ModelDeletedWhileExecuting,
            "parked destruction"
        );
    }


    /// A native unstarted task that has not blocked on its gate is not reclaimable yet.
    void CheckUnstartedReclamation() {
        Reset();
        Fixture fixture;
        fixture.Initialize();
        Require(
            fixture.provider.Destroy() == ExecutionDestroyResult::ProviderFailure &&
            ModelDeleteCalls == 0U,
            "refuse ready unstarted native task"
        );
        ModelTaskState = eBlocked;
        Require(
            fixture.provider.Destroy() == ExecutionDestroyResult::Succeeded &&
            !ModelDeletedWhileExecuting,
            "reclaim blocked private start gate"
        );
    }


    /// Completion wait and native parking consume one shared finite deadline, including tick wrap.
    void CheckSharedFiniteBudget(bool wrap) {
        Reset();
        Fixture fixture;
        fixture.Initialize();
        fixture.Start();
        ModelTick = wrap ? UINT32_MAX - 2U : 0U;
        const auto start = ModelTick;
        ModelCompletionTakeCost = 3U;
        ModelSuspendAtTick = static_cast<TickType_t>(start + 6U);
        ModelCompletion->count = 1U;
        ModelTaskState = eRunning;

        Require(
            fixture.provider.Join(
                WaitTimeout::ForNanoseconds(5000000U)
            ) == ExecutionJoinResult::TimedOut,
            "finite park deadline"
        );
        Require(
            static_cast<TickType_t>(ModelTick - start) == 5U &&
            ModelDelayCalls == 2U &&
            ModelDeleteCalls == 0U,
            "completion and parking share five ticks"
        );

        ModelTaskState = eSuspended;
        Require(
            fixture.provider.Join(WaitTimeout::NoWait()) == ExecutionJoinResult::Succeeded,
            "finite timeout preserves completion"
        );
        Require(
            fixture.provider.Destroy() == ExecutionDestroyResult::Succeeded,
            "finite retry destruction"
        );
    }


    /// Reinitialization cannot inherit completion from the previous native incarnation.
    void CheckFreshIncarnation() {
        Reset();
        Fixture fixture;
        fixture.Initialize();
        fixture.Start();
        ModelCompletion->count = 1U;
        ModelTaskState = eSuspended;
        Require(
            fixture.provider.Join(WaitTimeout::NoWait()) == ExecutionJoinResult::Succeeded &&
            fixture.provider.Destroy() == ExecutionDestroyResult::Succeeded,
            "first incarnation"
        );
        fixture.Initialize();
        fixture.Start();
        ModelTaskState = eSuspended;
        Require(
            fixture.provider.Join(WaitTimeout::NoWait()) == ExecutionJoinResult::TimedOut,
            "fresh incarnation requires fresh completion"
        );
    }

} // NativeExecutionLifecycleModel


/// Runs actual source against a deliberately controlled SDK MODEL, never native task qualification.
int main() {
    using namespace NativeExecutionLifecycleModel;
    CheckCompletionBeforeParking();
    CheckUnstartedReclamation();
    CheckSharedFiniteBudget(false);
    CheckSharedFiniteBudget(true);
    CheckFreshIncarnation();
}
