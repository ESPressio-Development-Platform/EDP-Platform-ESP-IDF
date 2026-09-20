#include <Arduino.h>

#include <array>
#include <cstddef>

#include <ESPressio_Platform_ESP_IDF.hpp>

namespace Demo {

    using Provider = ESPressio::Platform::ESPIDF::Execution::ExecutionContextProvider;
    using Contract = ESPressio::Platform::Execution::Detail::ExecutionContextProviderTraits<Provider>;
    using Properties = typename Contract::Properties;

    /// Demonstration stack reservation in bytes.
    inline constexpr std::size_t StackBytes = 4096U;

    static_assert(
        StackBytes %
        Properties::template Value<
            ESPressio::Platform::Execution::StackAllocationGranularityBytes
        > == 0U,
        "Demo stack size must satisfy the selected provider granularity"
    );


    // Static execution storage.

    /// Native control storage supplied to the provider.
    alignas(
        Properties::template Value<
            ESPressio::Platform::Execution::ControlStorageAlignment
        >
    ) std::array<
        std::byte,
        Properties::template Value<
            ESPressio::Platform::Execution::ControlStorageBytes
        >
    > ControlStorage{};

    /// Native stack storage supplied to the provider.
    alignas(
        Properties::template Value<
            ESPressio::Platform::Execution::StackStorageAlignment
        >
    ) std::array<
        std::byte,
        StackBytes
    > StackStorage{};

    /// Value updated by the demonstration execution context.
    volatile bool Executed = false;


    /// Runs once inside the selected native execution context.
    void Entry(
        void*
    ) noexcept {
        Executed = true;
    }


    /// Creates, starts, joins and destroys one core-affinity-capable execution context.
    int Run() noexcept {
        Provider provider;

        const ESPressio::Platform::Execution::ExecutionStorage storage {
            ControlStorage.data(),
            ControlStorage.size(),
            StackStorage.data(),
            StackStorage.size()
        };

        ESPressio::Platform::Execution::ExecutionConfiguration configuration;
        configuration.Priority = ESPressio::Platform::Execution::ExecutionPriority::Normal;
        configuration.Affinity =
            ESPressio::Platform::Execution::ProcessorAffinity::Specific(
                0U
            );
        configuration.Name = "EDPIDF";

        if (
            provider.Initialize(
                storage,
                configuration,
                &Entry,
                nullptr
            ) != ESPressio::Platform::Execution::ExecutionInitializationResult::Succeeded
        ) {
            return 1;
        }

        if (
            provider.Start() !=
            ESPressio::Platform::Execution::ExecutionStartResult::Succeeded
        ) {
            return 2;
        }

        if (
            provider.Join(
                ESPressio::Platform::Synchronization::WaitTimeout::Forever()
            ) != ESPressio::Platform::Execution::ExecutionJoinResult::Succeeded
        ) {
            return 3;
        }

        if (
            provider.Destroy() !=
            ESPressio::Platform::Execution::ExecutionDestroyResult::Succeeded
        ) {
            return 4;
        }

        return Executed ? 0 : 5;
    }

} // Demo


/// Runs the ESP-IDF execution-context demonstration once.
void setup() {
    static_cast<void>(
        Demo::Run()
    );
}

/// Leaves the demonstration idle.
void loop() {}
