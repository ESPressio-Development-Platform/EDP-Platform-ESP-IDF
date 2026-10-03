# EDP-Platform-ESP-IDF Developer Wiki

EDP-Platform-ESP-IDF provides concrete ESP-IDF-backed providers for abstract EDP-Platform, EDP-Memory and EDP-Security contracts.

The repository owns integrations whose implementation depends specifically on ESP-IDF, its FreeRTOS extensions, heap-capability APIs, hardware random source or bundled Mbed TLS. Portable C/C++ providers remain in EDP-Platform-Portable; generic FreeRTOS providers remain in EDP-Platform-FreeRTOS; platform-neutral security semantics remain in EDP-Security.

This Wiki is maintained on the same branch as the code. Source and repository `docs/` are normative; the Wiki is the comprehensive internal developer and reference layer.

## Public entry points

~~~cpp
#include <ESPressio_Platform_ESP_IDF.hpp>
~~~

Consumers requiring only the spin-lock provider may use:

~~~cpp
#include <ESPressio_Platform_ESP_IDF_SpinLock.hpp>
~~~

## Provider families

- Platform: execution context, spin lock and cryptographically suitable random bytes.
- Memory: internal, external-only and external-preferred raw resources.
- Security: AES-256-GCM, secure zeroization and provisioned Mesh V1 deployment-key derivation.

## Dependencies

Mandatory direct dependencies are EDP-Platform, EDP-Memory and EDP-Security. EDP-BoundedTopology is mandatory transitively through EDP-Memory.

During Mesh V1 feature integration the package manifest selects EDP-Security's `mesh_v1` branch; versions remain `0.1.0`.

## Navigation

Start with [Architecture](Architecture.md), then use [Composition](Composition.md) and [Dependency Contracts](Dependency-Contracts.md) for provider wiring. [Resources / Lifecycle / Concurrency](Resources-Lifecycle-Concurrency.md) describes bounded state and ownership. [Build / Test / Source](Build-Test-Source.md) records validation and source navigation. Every production header is indexed by [Reference Index](Reference-Index.md).
