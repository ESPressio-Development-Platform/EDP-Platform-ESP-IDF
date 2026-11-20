#include "TargetRecord.hpp"

#ifdef ARDUINO
#include <Arduino.h>

/// Runs qualification in the ordinary Arduino application task.
void setup() {
    Serial.begin(115200);
    delay(500);
    NativeBindingQualification::Run();
}

/// Yields after qualification completes.
void loop() {
    delay(1000);
}
#else
/// Runs qualification in the ordinary native application task.
extern "C" void app_main() {
    NativeBindingQualification::Run();
}
#endif
