#include <Arduino.h>
#include "Example.hpp"

/// Runs in the ordinary Arduino task.
void setup() {
    Serial.begin(115200);
    delay(500);
    CurrentContextBindingDemo::Run();
}

/// Yields through the application runtime.
void loop() {
    delay(1000);
}
