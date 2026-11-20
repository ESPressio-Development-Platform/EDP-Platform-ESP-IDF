#pragma once

#include <cstdint>

#define configSUPPORT_STATIC_ALLOCATION 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_eTaskGetState 1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define configMAX_PRIORITIES 25U
#define configTICK_RATE_HZ 1000U

using BaseType_t = int;
using UBaseType_t = unsigned;
using TickType_t = std::uint32_t;
using StackType_t = std::uint32_t;
struct StaticTask_t { unsigned unused = 0U; };
using TaskHandle_t = StaticTask_t*;
struct StaticSemaphore_t { unsigned count = 0U; unsigned role = 0U; };
using SemaphoreHandle_t = StaticSemaphore_t*;
inline constexpr BaseType_t pdTRUE = 1;
inline constexpr BaseType_t pdFALSE = 0;
inline constexpr BaseType_t tskNO_AFFINITY = -1;
inline constexpr TickType_t portMAX_DELAY = UINT32_MAX;

// These unscoped names deliberately model the external FreeRTOS C API, not library vocabulary.
enum eTaskState { eRunning, eReady, eBlocked, eSuspended, eDeleted, eInvalid };

inline eTaskState ModelTaskState = eReady;
inline TickType_t ModelTick = 0U;
inline unsigned ModelDelayCalls = 0U;
inline unsigned ModelDeleteCalls = 0U;
inline unsigned ModelCreateCalls = 0U;
inline unsigned ModelSemaphoreCreates = 0U;
inline TickType_t ModelCompletionTakeCost = 0U;
inline TickType_t ModelSuspendAtTick = UINT32_MAX;
inline SemaphoreHandle_t ModelCompletion = nullptr;
inline TaskHandle_t ModelCurrentTask = nullptr;
inline bool ModelDeletedWhileExecuting = false;

inline BaseType_t taskVALID_CORE_ID(BaseType_t core) noexcept {
    return core >= 0 && core < 2 ? pdTRUE : pdFALSE;
}
inline void taskYIELD() noexcept {}
