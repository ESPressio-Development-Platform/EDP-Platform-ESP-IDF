#include "Example.hpp"
#include <Arduino.h>

/// Runs after framework initialization.
void setup() { NativeStrongExample::Run(); }

/// Keeps the completed image observable.
void loop() { delay(1000); }
