# src/execution/ExecutionContextProvider.hpp

**Primary classification:** PUBLIC PROVIDER / EXTENSION API

**Source baseline:** `3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5/src/execution/ExecutionContextProvider.hpp)

## Direct includes

- `cstdint`
- `limits`
- `freertos/FreeRTOS.h`
- `freertos/idf_additions.h`
- `freertos/semphr.h`
- `freertos/task.h`
- `ESPressio_Platform.hpp`
- `../detail/ExecutionPriority.hpp`
- `../detail/ESPIdfWait.hpp`

## Documented declarations

### `StackTelemetryAvailable`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Indicates that this ESP-IDF configuration exposes stack high-water telemetry.

```cpp
inline constexpr bool StackTelemetryAvailable = true;
```

### `StackTelemetryAvailable`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Indicates that this ESP-IDF configuration does not expose stack high-water telemetry.

```cpp
inline constexpr bool StackTelemetryAvailable = false;
```

### `ExecutionContextProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

ESP-IDF static execution-context provider with processor-affinity support.

```cpp
class ExecutionContextProvider final : public Framework::Provider<
        ESPressio::Platform::Domain,
```

### `_handle`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Native ESP-IDF FreeRTOS task handle while initialized.

```cpp
TaskHandle_t _handle = nullptr;
```

### `_entry`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

User entry function invoked after Start.

```cpp
ESPressio::Platform::Execution::ExecutionEntry _entry = nullptr;
```

### `_parameter`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

User parameter passed to the entry function.

```cpp
void* _parameter = nullptr;
```

### `_initialized`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether the execution context was initialized.

```cpp
bool _initialized = false;
```

### `_started`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether Start has been accepted.

```cpp
bool _started = false;
```

### `_joined`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether the owner has successfully joined the completed execution.

```cpp
bool _joined = false;
```

### `StaticSemaphore_t _startStorage{};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Static storage for the start gate.

```cpp
StaticSemaphore_t _startStorage{};
```

### `StaticSemaphore_t _completionStorage{};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Static storage for the completion signal.

```cpp
StaticSemaphore_t _completionStorage{};
```

### `_startSignal`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Start-gate semaphore.

```cpp
SemaphoreHandle_t _startSignal;
```

### `_completionSignal`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Completion semaphore.

```cpp
SemaphoreHandle_t _completionSignal;
```

### `NativeEntry`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Bridges the ESP-IDF FreeRTOS task entry into the ESPressio execution function.

```cpp
static void NativeEntry(
                void* parameter
            ) noexcept
```

### `IsAligned`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Reports whether an address satisfies a required alignment.

```cpp
static bool IsAligned(
                const void* address,
                std::size_t alignment
            ) noexcept
```

### `ExecutionContextProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Creates an uninitialized ESP-IDF execution provider.

```cpp
ExecutionContextProvider() noexcept :
                _startSignal(
                    xSemaphoreCreateBinaryStatic(
                        &_startStorage
                    )
                ),
```

### `ExecutionContextProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents copying native execution state.

```cpp
ExecutionContextProvider(const ExecutionContextProvider&) = delete;
```

### `operator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents copy assignment of native execution state.

```cpp
ExecutionContextProvider& operator =(const ExecutionContextProvider&) = delete;
```

### `ExecutionContextProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents moving native execution state.

```cpp
ExecutionContextProvider(ExecutionContextProvider&&) = delete;
```

### `operator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents move assignment of native execution state.

```cpp
ExecutionContextProvider& operator =(ExecutionContextProvider&&) = delete;
```

### `Initialize`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Binds caller-owned storage and creates one start-gated ESP-IDF task.

```cpp
ESPressio::Platform::Execution::ExecutionInitializationResult Initialize(
                const ESPressio::Platform::Execution::ExecutionStorage& storage,
                const ESPressio::Platform::Execution::ExecutionConfiguration& configuration,
                ESPressio::Platform::Execution::ExecutionEntry entry,
                void* parameter
            ) noexcept
```

### `Start`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Releases the private start gate and permits the native task to enter user code.

```cpp
ESPressio::Platform::Execution::ExecutionStartResult Start() noexcept
```

### `Join`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Waits until the user execution entry has returned and records successful ownership join.

```cpp
ESPressio::Platform::Execution::ExecutionJoinResult Join(
                ESPressio::Platform::Synchronization::WaitTimeout timeout
            ) noexcept
```

### `Destroy`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Destroys an unstarted context or a context whose completed execution was joined.

```cpp
ESPressio::Platform::Execution::ExecutionDestroyResult Destroy() noexcept
```

### `IsCurrentContext`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Reports whether the caller is currently executing inside this native context.

```cpp
bool IsCurrentContext() const noexcept
```

### `GetStackTelemetry`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Returns the minimum free stack bytes when ESP-IDF stack telemetry is enabled.

```cpp
ESPressio::Platform::Execution::ExecutionStackTelemetry GetStackTelemetry() const noexcept
```

### `Yield`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Yields the current ESP-IDF FreeRTOS execution context.

```cpp
static void Yield() noexcept
```

### `ExecutionContextContract`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Compile-time validation of the ESP-IDF ExecutionContext provider.

```cpp
using ExecutionContextContract = ESPressio::Platform::Execution::Detail::ExecutionContextProviderTraits<ExecutionContextProvider>;
```

## Preprocessor controls in this header

- `#if defined(INCLUDE_uxTaskGetStackHighWaterMark) && ( INCLUDE_uxTaskGetStackHighWaterMark == 1 )`
- `#if ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( INCLUDE_vTaskDelete == 1 ) && ( INCLUDE_vTaskSuspend == 1 ) && ( INCLUDE_xTaskGetCurrentTaskHandle == 1 )`
- `#if defined(INCLUDE_uxTaskGetStackHighWaterMark) && ( INCLUDE_uxTaskGetStackHighWaterMark == 1 )`
