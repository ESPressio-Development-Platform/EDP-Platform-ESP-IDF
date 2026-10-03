# Compiler Definitions and Conditional Compilation

The repository defines no public EDP-owned compiler configuration switch.

## Mbed TLS configuration selection

`Aes256GcmProvider.hpp`, `DeploymentKeySourceProvider.hpp` and `SecretZeroizationProvider.hpp` contain:

~~~cpp
#if defined(ESP_PLATFORM) && !defined(MBEDTLS_CONFIG_FILE)
#define MBEDTLS_CONFIG_FILE "mbedtls/esp_config.h"
#endif
~~~

`ESP_PLATFORM` is supplied by ESP-IDF. `MBEDTLS_CONFIG_FILE` is an Mbed TLS integration macro. The repository supplies the ESP-IDF default only when the build has not already selected a configuration; neither macro is an EDP application-policy knob.

## SDK configuration

The deployment-key ESP-IDF demo's `sdkconfig.defaults` sets `CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y` to match the ESP32 Dev Module target and avoid generated-config flash-size drift.

The deployment-key provider does **not** require `CONFIG_MBEDTLS_HKDF_C`. It uses the SHA-256 streaming API to avoid the allocation-capable generic HKDF/message-digest setup.

Other ESP-IDF/FreeRTOS `CONFIG_*` and port macros may conditionally expose native facilities such as stack telemetry. They remain SDK-owned inputs. Concrete provider properties must truthfully reflect the compiled native capabilities.

> Build/conditional-compilation audit baseline: `f7c5667e8359e29d9c2a23f4aa44b8761ca6ac60` on `mesh_v1`.
