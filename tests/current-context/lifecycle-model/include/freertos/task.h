#pragma once

#include "FreeRTOS.h"

inline TickType_t xTaskGetTickCount() noexcept {
    return ModelTick;
}
inline TaskHandle_t xTaskGetCurrentTaskHandle() noexcept {
    return ModelCurrentTask;
}
inline eTaskState eTaskGetState(TaskHandle_t) noexcept {
    return ModelTaskState;
}
inline void vTaskDelay(TickType_t ticks) noexcept {
    ++ModelDelayCalls;
    ModelTick += ticks;
    if (ModelTick == ModelSuspendAtTick) {
        ModelTaskState = eSuspended;
    }
}
inline void vTaskSuspend(TaskHandle_t) noexcept {
    ModelTaskState = eSuspended;
}
inline void vTaskDelete(TaskHandle_t) noexcept {
    ++ModelDeleteCalls;
    ModelDeletedWhileExecuting = ModelTaskState == eRunning || ModelTaskState == eReady;
    ModelTaskState = eDeleted;
}
inline UBaseType_t uxTaskGetStackHighWaterMark(TaskHandle_t) noexcept {
    return 2048U;
}
inline TaskHandle_t xTaskCreateStaticPinnedToCore(
    void (*)(void*),
    const char*,
    std::uint32_t,
    void*,
    UBaseType_t,
    StackType_t*,
    StaticTask_t* storage,
    BaseType_t
) noexcept {
    ++ModelCreateCalls;
    ModelTaskState = eReady;
    return storage;
}
