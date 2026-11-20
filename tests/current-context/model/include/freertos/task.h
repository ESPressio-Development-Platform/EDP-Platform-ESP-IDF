#pragma once

#include "FreeRTOS.h"

/// Models the bounded SDK query; never a real kernel call.
inline int xTaskGetSchedulerState() noexcept {
    return ModelSchedulerState;
}
