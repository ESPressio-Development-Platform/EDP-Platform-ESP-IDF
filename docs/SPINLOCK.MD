# ESP-IDF SpinLock Provider

The provider uses `portMUX_TYPE` with ESP-IDF critical-section APIs.

Ordinary acquisition uses `portENTER_CRITICAL` and release uses `portEXIT_CRITICAL`.

Interrupt-context acquisition/release use the corresponding ISR critical-section APIs.

Holding this lock masks interrupts to the configured critical level and may spin while another core owns the mux. The protected scope must remain extremely short and must never block or yield.
