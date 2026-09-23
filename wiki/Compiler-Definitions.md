# Compiler Definitions and Conditional Compilation

No repository-defined compiler definitions were found in the audited PlatformIO/CMake/SDK configuration.

ESP-IDF and FreeRTOS `CONFIG_*`/port macros may conditionally expose native facilities such as stack telemetry, but they are supplied by the integrating SDK configuration. They must not be confused with EDP-owned public configuration switches. Concrete provider properties truthfully reflect the resulting native capability.

> Audited against `96282b7cb9d8f2cd16dc233d384166543402c7a4` on `main` across 11 build-configuration file(s).
