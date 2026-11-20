#include "TargetRecord.hpp"
#include <ESPressio_Platform_ESP_IDF_ExecutionContext.hpp>
#include <timing/EspTimerMonotonicTimebaseProvider.hpp>
#include <clock/MonotonicClockProvider.hpp>

#include <cstdio>
#include <cstdlib>
#include <driver/gptimer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

namespace NativeBindingQualification {

    using namespace ESPressio::Platform::Execution;

    /// Writes fixture diagnostics to the selected application console.
    /// @tparam TArguments Ordinary printf-format argument Types used only by the fixture.
    template<class... TArguments>
    void Print(
        const char* format,
        TArguments... arguments
    ) {
#ifdef ARDUINO
        Serial.printf(
            format,
            arguments...
        );
#else
        std::printf(
            format,
            arguments...
        );
#endif
    }

    /// Flushes the fixture's actual selected console before capture completes.
    void Flush() {
#ifdef ARDUINO
        Serial.flush();
#else
        std::fflush(stdout);
#endif
    }

    // Fixed test resources, retained until firmware termination.

    /// One explicit binding instance shared by ordinary test tasks.
    Binding binding;

    /// Controller descriptor remains alive during ISR and child task tests.
    const Record controllerRecord{1U};

    /// Every reused native incarnation receives a distinct test identity.
    const Record records[] = {
        {101U}, {102U}, {103U}, {104U},
        {105U}, {106U}, {107U}, {108U},
        {109U}, {110U}, {111U}, {112U},
        {113U}, {114U}, {115U}, {116U},
        {117U}, {118U}, {119U}, {120U}
    };

    /// Reused SDK TCB storage; reuse occurs only after controller deletion returns.
    StaticTask_t controls[2];

    /// Fixed 8192-byte stack budget per test native task.
    StackType_t stacks[2][8192U / sizeof(StackType_t)];

    /// Explicit ordinary-context test entry configuration.
    struct Entry final {

        /// The descriptor for this native incarnation.
        const Record* record;

        /// Controller notification bit.
        std::uint32_t bit;

        /// Ordinary task receiving phase completion notifications.
        TaskHandle_t controller;

        /// Whether this fixture deliberately exercises deletion without retirement.
        bool abrupt;

    };

    /// Mutated only after the previous incarnation can no longer resume.
    Entry entries[2];


    /// Keeps target assertions effective regardless of NDEBUG.
    void Require(
        bool condition,
        const char* label
    ) {
        if (!condition) {
            Print(
                "BINDING_HIL_FAIL %s\n",
                label
            );
            Flush();
            std::abort();
        }
    }


    /// Waits in ordinary controller code, outside the binding operation under test.
    void WaitBits(std::uint32_t expected) {
        std::uint32_t seen = 0U;
        while ((seen & expected) != expected) {
            std::uint32_t next = 0U;
            Require(
                xTaskNotifyWait(
                    0U,
                    UINT32_MAX,
                    &next,
                    pdMS_TO_TICKS(5000U)
                ) == pdTRUE,
                "notification timeout"
            );
            seen |= next;
        }
    }


    /// Proves stable nonexecution before deleting/reusing a cross-core static task.
    /// The fixture sends no further notification/resume after this observation.
    void AwaitParked(
        TaskHandle_t task,
        eTaskState expected
    ) {
        for (unsigned poll = 0U; poll < 1000U; ++poll) {
            if (eTaskGetState(task) == expected) {
                return;
            }
            vTaskDelay(1U);
        }
        Require(
            false,
            "native task did not reach stable parked state"
        );
    }


    /// Deletes only a task parked on an infinite wait without any future wake sender.
    void DeleteParkedTask(TaskHandle_t task) {
        AwaitParked(
            task,
            eBlocked
        );
        vTaskDelete(task);
    }


    /// Tests persistent ordinary task identity across yield, block and activation boundaries.
    void TaskEntry(void* opaque) {
        const auto& entry = *static_cast<const Entry*>(opaque);
        Require(
            binding.ReadCurrent().Status() == CurrentContextReadStatus::Unbound,
            "fresh native task TLS"
        );
        Require(
            binding.BindCurrent(*entry.record) == CurrentContextBindResult::Bound,
            "task bind"
        );

        for (unsigned round = 0U; round < 20000U; ++round) {
            Require(
                binding.ReadCurrent().GetIfBound() == entry.record &&
                ReadOtherTranslationUnit() == entry.record,
                "switch/TU isolation"
            );
            taskYIELD();
        }

        Require(
            xTaskNotify(
                entry.controller,
                entry.bit,
                eSetBits
            ) == pdPASS,
            "task ready"
        );
        if (entry.abrupt) {
            vTaskSuspend(nullptr);
            Require(
                binding.ReadCurrent().GetIfBound() == entry.record,
                "binding preserved across suspension"
            );
            xTaskNotify(
                entry.controller,
                entry.bit,
                eSetBits
            );
            for (;;) {
                vTaskSuspend(nullptr);
            }
        }

        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );

        Require(
            binding.ReadCurrent().GetIfBound() == entry.record,
            "persistent activation"
        );
        Require(
            binding.RetireCurrent(*entry.record) == CurrentContextRetireResult::Retired,
            "task retire"
        );
        Require(
            !binding.ReadCurrent().IsBound(),
            "empty after retire"
        );
        Print(
            "BINDING_TASK identity=%llu core=%d stack_min_bytes=%lu\n",
            static_cast<unsigned long long>(entry.record->identity),
            xPortGetCoreID(),
            static_cast<unsigned long>(uxTaskGetStackHighWaterMark(nullptr))
        );
        xTaskNotify(
            entry.controller,
            entry.bit,
            eSetBits
        );

        // The controller deletes this blocked static task before reusing its TCB/stack.
        for (;;) {
            ulTaskNotifyTake(
                pdTRUE,
                portMAX_DELAY
            );
        }
    }


    /// Defensive ISR rejection test, not an assertion that application ISR use is supported.
    bool TimerCallback(
        gptimer_handle_t,
        const gptimer_alarm_event_data_t*,
        void* user
    ) {
        auto controller = static_cast<TaskHandle_t>(user);
        const auto read = binding.ReadCurrent();
        const bool rejected =
            read.Status() == CurrentContextReadStatus::UnsupportedContext &&
            read.GetIfBound() == nullptr &&
            binding.BindCurrent(controllerRecord) == CurrentContextBindResult::UnsupportedContext &&
            binding.RetireCurrent(controllerRecord) == CurrentContextRetireResult::UnsupportedContext;
        BaseType_t wake = pdFALSE;
        xTaskNotifyFromISR(
            controller,
            rejected ? 4U : 8U,
            eSetBits,
            &wake
        );
        return wake == pdTRUE;
    }



    // Core migration qualification resources. All waits and notifications are fixture setup.

    /// Independent fixed stacks for two pinned load tasks, one unpinned task and its controller.
    StackType_t migrationStacks[4][8192U / sizeof(StackType_t)];

    /// Static native control blocks, never shared between simultaneously live tasks.
    StaticTask_t migrationControls[4];

    /// Pinned load task handles.
    TaskHandle_t hogs[2];

    /// Pinned core-zero migration controller.
    TaskHandle_t migrationController;

    /// Original application task receiving migration completion.
    TaskHandle_t applicationController;

    /// Firmware-lifetime descriptor for the same native task before and after migration.
    const Record migrationRecord{900U};


    /// Runs a finite high-priority load phase; no test can hold a core indefinitely.
    void HogEntry(void* opaque) {
        const auto core = reinterpret_cast<std::uintptr_t>(opaque);
        ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );
        if (core == 1U) {
            xTaskNotify(
                migrationController,
                16U,
                eSetBits
            );
        } else {
            xTaskNotifyGive(hogs[1]);
        }

        bool released = false;
        for (unsigned spin = 0U; spin < 500000U; ++spin) {
            if (ulTaskNotifyTake(pdTRUE, 0U) != 0U) {
                released = true;
                break;
            }
        }
        if (!released) {
            xTaskNotify(
                migrationController,
                64U,
                eSetBits
            );
        }
        for (;;) {
            ulTaskNotifyTake(
                pdTRUE,
                portMAX_DELAY
            );
        }
    }


    /// Is forced from core zero to core one without replacing its native task or descriptor.
    void MigratingEntry(void*) {
        Require(
            xPortGetCoreID() == 0,
            "migration initial core"
        );
        Require(
            binding.ReadCurrent().Status() == CurrentContextReadStatus::Unbound &&
            binding.BindCurrent(migrationRecord) == CurrentContextBindResult::Bound,
            "migration initial binding"
        );
        xTaskNotifyGive(hogs[0]);

        // Core-zero priority-three load preempts us and releases the core-one load.
        // This ready, unpinned priority-two task can then resume only on core one.
        Require(
            xPortGetCoreID() == 1 &&
            binding.ReadCurrent().GetIfBound() == &migrationRecord &&
            ReadOtherTranslationUnit() == &migrationRecord,
            "same native task migrated with TLS"
        );
        xTaskNotifyGive(hogs[0]);
        Require(
            binding.RetireCurrent(migrationRecord) == CurrentContextRetireResult::Retired,
            "migration retire"
        );
        xTaskNotify(
            migrationController,
            32U,
            eSetBits
        );
        for (;;) {
            ulTaskNotifyTake(
                pdTRUE,
                portMAX_DELAY
            );
        }
    }


    /// Controls migration from core zero even when the Arduino application task is on core one.
    void MigrationControllerEntry(void*) {
        migrationController = xTaskGetCurrentTaskHandle();
        for (unsigned core = 0U; core < 2U; ++core) {
            hogs[core] = xTaskCreateStaticPinnedToCore(
                HogEntry,
                "binding-load",
                sizeof(migrationStacks[core]),
                reinterpret_cast<void*>(static_cast<std::uintptr_t>(core)),
                3U,
                migrationStacks[core],
                &migrationControls[core],
                static_cast<BaseType_t>(core)
            );
            Require(
                hogs[core] != nullptr,
                "load task creation"
            );
        }
        xTaskNotifyGive(hogs[1]);
        WaitBits(16U);
        const auto migrating = xTaskCreateStaticPinnedToCore(
            MigratingEntry,
            "binding-migrate",
            sizeof(migrationStacks[2]),
            nullptr,
            2U,
            migrationStacks[2],
            &migrationControls[2],
            tskNO_AFFINITY
        );
        Require(
            migrating != nullptr,
            "unpinned task creation"
        );
        WaitBits(32U);
        DeleteParkedTask(migrating);
        for (const auto hog : hogs) {
            DeleteParkedTask(hog);
        }
        xTaskNotify(
            applicationController,
            128U,
            eSetBits
        );
        for (;;) {
            ulTaskNotifyTake(
                pdTRUE,
                portMAX_DELAY
            );
        }
    }


    /// Forces one actual same-incarnation migration and verifies its borrowed pointer identity.
    void CheckMigration() {
        applicationController = xTaskGetCurrentTaskHandle();
        const auto controller = xTaskCreateStaticPinnedToCore(
            MigrationControllerEntry,
            "binding-migctl",
            sizeof(migrationStacks[3]),
            nullptr,
            2U,
            migrationStacks[3],
            &migrationControls[3],
            0
        );
        Require(
            controller != nullptr,
            "migration controller creation"
        );
        WaitBits(128U);
        DeleteParkedTask(controller);
        Print("BINDING_MIGRATION core0_to_core1=PASS identity=900\n");
    }


    // Actual native execution-provider reclamation regression.

    /// Native task/control storage reused only after the actual provider proves parked termination.
    StaticTask_t executionControl;

    /// Fixed stack supplied to the actual native owner.
    StackType_t executionStack[8192U / sizeof(StackType_t)];

    /// Explicit callback arguments remain alive through successful native join/destruction.
    struct ExecutionArguments final {

        /// The native provider whose context must be current.
        ESPressio::Platform::ESPIDF::Execution::ExecutionContextProvider* provider;

        /// Fresh caller-owned incarnation descriptor.
        const Record* record;

    };


    /// Binds/reads/retires inside the real provider's user-entry boundary.
    void ActualOwnerEntry(void* opaque) noexcept {
        const auto& arguments = *static_cast<const ExecutionArguments*>(opaque);
        Require(
            arguments.provider->IsCurrentContext() &&
            binding.ReadCurrent().Status() == CurrentContextReadStatus::Unbound,
            "actual native owner fresh context"
        );
        Require(
            binding.BindCurrent(*arguments.record) == CurrentContextBindResult::Bound &&
            ReadOtherTranslationUnit() == arguments.record,
            "actual native owner binding"
        );
        Require(
            binding.RetireCurrent(*arguments.record) == CurrentContextRetireResult::Retired,
            "actual native owner retirement"
        );
    }


    /// Stresses the actual completion/parking/join/destruction path across both cores and static reuse.
    void CheckActualExecutionOwner() {
        using Provider = ESPressio::Platform::ESPIDF::Execution::ExecutionContextProvider;
        using ESPressio::Platform::Synchronization::WaitTimeout;
        Provider provider;

        ExecutionStorage storage{};
        storage.ControlAddress = &executionControl;
        storage.ControlBytes = sizeof(executionControl);
        storage.StackAddress = executionStack;
        storage.StackBytes = sizeof(executionStack);

        for (unsigned incarnation = 0U; incarnation < 100U; ++incarnation) {
            const Record record{2000U + incarnation};
            ExecutionArguments arguments{
                &provider,
                &record
            };
            ExecutionConfiguration configuration{};
            configuration.Name = "binding-owner";
            configuration.Priority = ExecutionPriority::Low;
            configuration.Affinity = ProcessorAffinity::Specific(incarnation % 2U);
            Require(
                provider.Initialize(
                    storage,
                    configuration,
                    ActualOwnerEntry,
                    &arguments
                ) == ExecutionInitializationResult::Succeeded,
                "actual owner initialize"
            );
            Require(
                provider.Start() == ExecutionStartResult::Succeeded,
                "actual owner start"
            );
            Require(
                provider.Join(
                    WaitTimeout::ForNanoseconds(2000000000ULL)
                ) == ExecutionJoinResult::Succeeded,
                "actual owner parked join"
            );
            Require(
                provider.Destroy() == ExecutionDestroyResult::Succeeded,
                "actual owner synchronous storage reclamation"
            );
        }
        Print("BINDING_ACTUAL_EXECUTION_OWNER_PASS incarnations=100 cores=2\n");
    }


    // One canonical monotonic clock belongs to this qualification application.

    /// Reuses the existing SDK-owned monotonic timebase without creating timer hardware.
    const ESPressio::Platform::ESPIDF::Timing::EspTimerMonotonicTimebaseProvider timebase;

    /// Sole canonical clock for the fixture's elapsed lookup measurements.
    const ESPressio::Clock::MonotonicClockProvider<
        ESPressio::Platform::ESPIDF::Timing::EspTimerMonotonicTimebaseProvider
    > clock(timebase);


    /// Measures warm carrier lookup plus loop/check cost, alongside a matched empty-loop baseline.
    /// Values are observations in nominal canonical nanoseconds, never worst-case guarantees.
    void MeasureLookup(bool expectBound) {
        constexpr unsigned iterations = 100000U;

        for (unsigned sample = 0U; sample < 7U; ++sample) {
            const auto baselineStart = clock.Now().Nanoseconds();
            for (unsigned iteration = 0U; iteration < iterations; ++iteration) {
                __asm__ __volatile__("" ::: "memory");
            }
            const auto baselineEnd = clock.Now().Nanoseconds();

            unsigned matched = 0U;
            const auto lookupStart = clock.Now().Nanoseconds();
            for (unsigned iteration = 0U; iteration < iterations; ++iteration) {
                __asm__ __volatile__("" ::: "memory");
                const auto* record = binding.ReadCurrent().GetIfBound();
                matched += record == (expectBound ? &controllerRecord : nullptr) ? 1U : 0U;
            }
            const auto lookupEnd = clock.Now().Nanoseconds();

            Require(
                matched == iterations &&
                baselineEnd >= baselineStart &&
                lookupEnd >= lookupStart,
                "lookup benchmark identity and monotonicity"
            );
            Print(
                "BINDING_LOOKUP_SAMPLE bound=%u sample=%u iterations=%u total_ns=%llu baseline_ns=%llu\n",
                expectBound ? 1U : 0U,
                sample,
                iterations,
                static_cast<unsigned long long>(lookupEnd - lookupStart),
                static_cast<unsigned long long>(baselineEnd - baselineStart)
            );
        }
    }

    void Run() {
        Require(
            binding.ReadCurrent().Status() == CurrentContextReadStatus::Unbound,
            "controller initially unbound"
        );
        Require(
            binding.BindCurrent(controllerRecord) == CurrentContextBindResult::Bound,
            "controller bind"
        );
        Require(
            binding.BindCurrent(records[0]) == CurrentContextBindResult::AlreadyBound,
            "duplicate bind"
        );
        Require(
            binding.RetireCurrent(records[0]) == CurrentContextRetireResult::DifferentBinding,
            "mismatched retire"
        );
        const auto controller = xTaskGetCurrentTaskHandle();

        gptimer_config_t timerConfig{};
        timerConfig.clk_src = GPTIMER_CLK_SRC_DEFAULT;
        timerConfig.direction = GPTIMER_COUNT_UP;
        timerConfig.resolution_hz = 1000000U;
        gptimer_handle_t timer = nullptr;
        Require(
            gptimer_new_timer(
                &timerConfig,
                &timer
            ) == ESP_OK,
            "timer setup"
        );
        gptimer_event_callbacks_t callbacks{};
        callbacks.on_alarm = TimerCallback;
        Require(
            gptimer_register_event_callbacks(
                timer,
                &callbacks,
                controller
            ) == ESP_OK,
            "ISR callback setup"
        );
        gptimer_alarm_config_t alarm{};
        alarm.alarm_count = 1000U;
        Require(
            gptimer_set_alarm_action(
                timer,
                &alarm
            ) == ESP_OK &&
            gptimer_enable(timer) == ESP_OK &&
            gptimer_start(timer) == ESP_OK,
            "timer start"
        );
        WaitBits(4U);
        Require(
            gptimer_stop(timer) == ESP_OK &&
            gptimer_disable(timer) == ESP_OK &&
            gptimer_del_timer(timer) == ESP_OK,
            "timer cleanup"
        );
        Require(
            binding.ReadCurrent().GetIfBound() == &controllerRecord,
            "ISR preserved interrupted binding"
        );

        for (unsigned incarnation = 0U; incarnation < 10U; ++incarnation) {
            TaskHandle_t tasks[2]{};
            for (unsigned slot = 0U; slot < 2U; ++slot) {
                entries[slot] = Entry{
                    &records[incarnation * 2U + slot],
                    1U << slot,
                    controller,
                    incarnation == 8U
                };
                tasks[slot] = xTaskCreateStaticPinnedToCore(
                    TaskEntry,
                    "binding-qual",
                    sizeof(stacks[slot]),
                    &entries[slot],
                    2U,
                    stacks[slot],
                    &controls[slot],
                    static_cast<BaseType_t>(slot)
                );
                Require(
                    tasks[slot] != nullptr,
                    "static task creation"
                );
            }
            WaitBits(3U);
            Require(
                binding.ReadCurrent().GetIfBound() == &controllerRecord,
                "controller isolation"
            );

            if (incarnation == 8U) {
                // Deliberate deletion of bound blocked tasks. No gateway flag is cleared.
                for (const auto task : tasks) {
                    AwaitParked(
                        task,
                        eSuspended
                    );
                    vTaskResume(task);
                }
                WaitBits(3U);
                for (const auto task : tasks) {
                    AwaitParked(
                        task,
                        eSuspended
                    );
                    vTaskDelete(task);
                }
            } else {
                for (const auto task : tasks) {
                    xTaskNotifyGive(task);
                }
                WaitBits(3U);
                for (const auto task : tasks) {
                    DeleteParkedTask(task);
                }
            }
        }

        CheckMigration();
        CheckActualExecutionOwner();
        MeasureLookup(true);

        Require(
            binding.RetireCurrent(controllerRecord) == CurrentContextRetireResult::Retired,
            "controller retirement"
        );
        MeasureLookup(false);
        Print(
            "BINDING_HIL_PASS target=%s incarnations=20 reads_per_task=20000 ISR=rejected migration=PASS threading_owner=NOT_RUN\n",
            CONFIG_IDF_TARGET
        );
        Flush();
    }

} // NativeBindingQualification
