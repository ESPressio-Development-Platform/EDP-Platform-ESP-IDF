#pragma once

#include "FreeRTOS.h"

inline SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t* storage) noexcept {
    storage->count = 0U;
    storage->role = ++ModelSemaphoreCreates;
    if (storage->role == 2U) {
        ModelCompletion = storage;
    }
    return storage;
}
inline BaseType_t xSemaphoreTake(
    SemaphoreHandle_t handle,
    TickType_t ticks
) noexcept {
    if (handle->count != 0U) {
        --handle->count;
        if (handle == ModelCompletion) {
            ModelTick += ModelCompletionTakeCost;
        }
        return pdTRUE;
    }
    if (ticks != portMAX_DELAY) {
        ModelTick += ticks;
    }
    return pdFALSE;
}
inline BaseType_t xSemaphoreGive(SemaphoreHandle_t handle) noexcept {
    ++handle->count;
    return pdTRUE;
}
