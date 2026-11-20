cmake_minimum_required(VERSION 3.20)
project(NativeCurrentContextBindingModel LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)
if(NOT EDP_PLATFORM_SOURCE_DIR OR NOT EDP_SYSTEM_SOURCE_DIR)
    message(FATAL_ERROR "Supply the Platform and System source owners")
endif()
add_executable(current_context_binding_model
    CurrentContextBindingModelTests.cpp OtherTranslationUnit.cpp)
target_include_directories(current_context_binding_model PRIVATE
    model/include ../../src ${EDP_PLATFORM_SOURCE_DIR}/src ${EDP_SYSTEM_SOURCE_DIR}/src)
# SDK MODEL only: target macro never qualifies actual Xtensa execution.
target_compile_definitions(current_context_binding_model PRIVATE __XTENSA__=1)
target_compile_options(current_context_binding_model PRIVATE
    -Wall -Wextra -Wpedantic -Werror -UNDEBUG)
target_link_libraries(current_context_binding_model PRIVATE Threads::Threads)
enable_testing()
add_test(NAME current_context_binding_model COMMAND current_context_binding_model)

add_executable(native_execution_lifecycle_model NativeExecutionLifecycleModelTests.cpp)
target_include_directories(native_execution_lifecycle_model PRIVATE
    lifecycle-model/include ../../src ${EDP_PLATFORM_SOURCE_DIR}/src ${EDP_SYSTEM_SOURCE_DIR}/src)
target_compile_options(native_execution_lifecycle_model PRIVATE
    -Wall -Wextra -Wpedantic -Werror -UNDEBUG)
add_test(NAME native_execution_lifecycle_model COMMAND native_execution_lifecycle_model)
