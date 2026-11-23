#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <source_location>
#include <ESPressio_Platform_ESP_IDF_AtomicWord32.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace NativeStrongExample {

    /// Existing native provider, with optional strong ordering on the same four-byte Word.
    using Provider = ESPressio::Platform::ESPIDF::Concurrency::AtomicWord32Provider;

    /// Qualified internal-DRAM atomic storage.
    using Word = Provider::Word;

    /// Fixed stress count, independent of probe implementation work bounds.
    inline constexpr unsigned Rounds = 10000U;

    /// Two independent protocol words, used exclusively through strong operations.
    inline std::array<Word, 2U> Values;

    /// Observed opposite-word values, published through native completion notifications.
    inline std::array<std::uint32_t, 2U> Results{};

    /// Caller-owned helper stacks, each explicitly 4096 bytes.
    inline StackType_t Stacks[2U][4096U / sizeof(StackType_t)];

    /// Caller-owned native control blocks.
    inline StaticTask_t Controls[2U];

    /// Helpers pinned on separate cores.
    inline TaskHandle_t Tasks[2U];

    /// Sole test/control owner, outside the atomic operations under test.
    inline TaskHandle_t Controller = nullptr;

    /// Fails with an assertion effective under NDEBUG.
    inline void Require(
        bool condition,
        const std::source_location location = std::source_location::current()
    ) noexcept {
        if (!condition) {
            std::printf(
                "NATIVE_STRONG_ATOMIC_FAIL line=%u function=%s\n",
                location.line(),
                location.function_name()
            );
            std::fflush(stdout);
            std::abort();
        }
    }

    /// Per-core store/load pair; native signals only coordinate independent test rounds.
    inline void Entry(void* opaque) noexcept {
        const auto core = reinterpret_cast<std::uintptr_t>(opaque);
        for (unsigned round = 0U; round < Rounds; ++round) {
            Require(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5000U)) != 0U);
            Require(xPortGetCoreID() == static_cast<int>(core));
            Values[core].StoreSequentiallyConsistent(1U);
            Results[core] = Values[1U - core].LoadSequentiallyConsistent();
            Require(xTaskNotify(
                Controller,
                1U << core,
                eSetBits
            ) == pdPASS);
        }
        for (;;) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }
    }

    /// Proves stable blocked state before deleting a fixture helper with no future wake sender.
    inline void DeleteParked(TaskHandle_t task) noexcept {
        for (unsigned poll = 0U; poll < 1000U; ++poll) {
            if (eTaskGetState(task) == eBlocked) {
                vTaskDelete(task);
                return;
            }
            vTaskDelay(1U);
        }
        Require(false);
    }

    /// Constructs internal words, coordinates cross-core pairs and rejects the forbidden zero/zero result.
    inline void Run() noexcept {
        static_assert(sizeof(Word) == 4U && alignof(Word) == 4U);
        static_assert(sizeof(ESPressio::Platform::Concurrency::Detail::SequentiallyConsistentAtomicWord32ProviderTraits<Provider>) > 0U);
        Controller = xTaskGetCurrentTaskHandle();
        for (unsigned core = 0U; core < 2U; ++core) {
            Tasks[core] = xTaskCreateStaticPinnedToCore(
                Entry,
                "atomic-strong",
                sizeof(Stacks[core]),
                reinterpret_cast<void*>(static_cast<std::uintptr_t>(core)),
                2U,
                Stacks[core],
                &Controls[core],
                static_cast<BaseType_t>(core)
            );
            Require(Tasks[core] != nullptr);
        }
        for (unsigned round = 0U; round < Rounds; ++round) {
            for (auto& word : Values) {
                word.StoreSequentiallyConsistent(0U);
            }
            xTaskNotifyGive(Tasks[0U]);
            xTaskNotifyGive(Tasks[1U]);
            std::uint32_t seen = 0U;
            while (seen != 3U) {
                std::uint32_t next = 0U;
                Require(xTaskNotifyWait(
                    0U,
                    UINT32_MAX,
                    &next,
                    pdMS_TO_TICKS(5000U)
                ) == pdTRUE);
                seen |= next;
            }
            Require(Results[0U] != 0U || Results[1U] != 0U);
        }
        for (const auto task : Tasks) {
            DeleteParked(task);
        }
        std::printf(
            "NATIVE_STRONG_ATOMIC_PASS rounds=%u words_bytes=%u forbidden=0 cores=0,1\n\n",
            Rounds,
            static_cast<unsigned>(sizeof(Values))
        );
        std::fflush(stdout);
    }

} // NativeStrongExample

/// Retained compiler-output inspection seam; actual provider operation, no wrapper loop.
extern "C" __attribute__((noinline, used)) std::uint32_t NativeStrongLoad(
    const NativeStrongExample::Word& word
) noexcept {
    return word.LoadSequentiallyConsistent();
}

/// Retained compiler-output inspection seam; actual provider operation, no wrapper loop.
extern "C" __attribute__((noinline, used)) void NativeStrongStore(
    NativeStrongExample::Word& word,
    std::uint32_t value
) noexcept {
    word.StoreSequentiallyConsistent(value);
}
