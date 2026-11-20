#include "Example.hpp"

/// Runs after the native scheduler starts.
extern "C" void app_main() {
    CurrentContextBindingDemo::Run();
}
