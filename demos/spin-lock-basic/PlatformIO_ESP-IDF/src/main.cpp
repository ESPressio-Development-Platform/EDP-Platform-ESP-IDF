#include <ESPressio_Platform_ESP_IDF.hpp>

namespace Demo {

    /// Acquires and releases the ESP-IDF spin lock once.
    int Run() noexcept {
        ESPressio::Platform::ESPIDF::Synchronization::SpinLockProvider spinLock;

        spinLock.Acquire();

        return spinLock.Release() ==
            ESPressio::Platform::Synchronization::SpinLockReleaseResult::Released
                ? 0
                : 1;
    }

} // Demo

/// Runs the ESP-IDF spin-lock demonstration once.
extern "C" void app_main() {
    static_cast<void>(
        Demo::Run()
    );
}
