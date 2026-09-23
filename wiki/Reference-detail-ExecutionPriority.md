# src/detail/ExecutionPriority.hpp

**Primary classification:** PRIVATE IMPLEMENTATION

**Source baseline:** `3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/3ea9e7b6ed6cf163defd6ff3b19d1be6b87552e5/src/detail/ExecutionPriority.hpp)

## Direct includes

- `freertos/FreeRTOS.h`
- `ESPressio_Platform.hpp`

## Documented declarations

### `NativePriorityFor`

**Classification:** PRIVATE IMPLEMENTATION

Maps one portable ESPressio execution-priority level onto the configured native FreeRTOS priority range.

The mapping is monotonic. Critical uses the highest native priority, Low avoids the
idle priority whenever the native scheduler exposes at least one non-idle level, and
small native ranges may intentionally collapse adjacent ESPressio levels.

```cpp
constexpr UBaseType_t NativePriorityFor(
        ESPressio::Platform::Execution::ExecutionPriority priority
    ) noexcept
```

