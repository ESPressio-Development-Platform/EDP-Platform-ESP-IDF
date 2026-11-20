# Native current-context binding reference

Production SDK owner; no Profiling or Threading source dependency.

| Source | Symbols |
|---|---|
| [Public umbrella](../src/ESPressio_Platform_ESP_IDF_CurrentContextBinding.hpp) | Includes only the native binding realization; unrelated SDK/security providers remain unselected. |
| [CurrentContextBindingProvider.hpp](../src/execution/CurrentContextBindingProvider.hpp) | ESPIDF::Execution::CurrentContextBindingProvider<TContextRecord>; CurrentContextBinding composition offer and all four true properties; ContextRecord/ReadResult aliases; default constructor, four deleted copy/move members; BindCurrent and two deleted rvalue overloads; const ReadCurrent; RetireCurrent. |
| [CurrentContextBindingProvider.hpp](../src/execution/CurrentContextBindingProvider.hpp) | Private inline static constinit thread_local _current; private static IsOrdinaryTaskContext; target/THREADPTR/C++20 rejection guards. |

[Usage, outcomes, lifetimes, query cost and qualification](../docs/CURRENT-CONTEXT-BINDING.MD).

## Maintained tests

- [CMake](../tests/current-context/CMakeLists.txt): EDP_PLATFORM_SOURCE_DIR/EDP_SYSTEM_SOURCE_DIR; actual-provider host SDK MODEL, with explicitly fake target headers. Defining __XTENSA__ in this fixture is not target qualification.
- [ModelRecord.hpp](../tests/current-context/ModelRecord.hpp): Record::identity, Provider alias, ReadFromOtherTranslationUnit.
- [OtherTranslationUnit.cpp](../tests/current-context/OtherTranslationUnit.cpp): reads the exact shared inline TLS specialization in a separate translation unit.
- [CurrentContextBindingModelTests.cpp](../tests/current-context/CurrentContextBindingModelTests.cpp): Require, CheckOperations, model OwnerScope (_provider/_record, constructor/destructor, deleted copying), CheckUnwinding, CheckThreadIsolation and main. Oracles remain enabled in Release.
- Model [sdkconfig.h](../tests/current-context/model/include/sdkconfig.h) and [core-isa.h](../tests/current-context/model/include/xtensa/config/core-isa.h): target/THREADPTR macros.
- Model [FreeRTOS.h](../tests/current-context/model/include/freertos/FreeRTOS.h) and [task.h](../tests/current-context/model/include/freertos/task.h): ModelInIsr, ModelSchedulerState, three scheduler constants, xPortInIsrContext, xTaskGetSchedulerState. Fake fixtures only.
- [Consumer demo](../demos/current-context-binding/README.MD): three surfaces with ContextRecord (identity/slot), Binding alias, explicit binding instance/applicationContext and Run; ordinary task wrappers. Not actual Threading integration.
