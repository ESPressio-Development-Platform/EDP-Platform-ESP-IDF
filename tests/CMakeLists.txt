cmake_minimum_required(VERSION 3.16)

project(EDPPlatformESPIDFMemoryHostTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED EDP_PLATFORM_SOURCE_DIR)
    message(FATAL_ERROR "EDP_PLATFORM_SOURCE_DIR must point at an EDP-Platform checkout")
endif()

if(NOT DEFINED EDP_SYSTEM_SOURCE_DIR)
    message(FATAL_ERROR "EDP_SYSTEM_SOURCE_DIR must point at an EDP-System checkout")
endif()

if(NOT DEFINED EDP_MEMORY_SOURCE_DIR)
    message(FATAL_ERROR "EDP_MEMORY_SOURCE_DIR must point at an EDP-Memory checkout")
endif()

add_executable(
    espidf_memory_provider_host_tests
    host/MemoryResourceProviderHostTests.cpp
)

target_include_directories(
    espidf_memory_provider_host_tests
    PRIVATE
        host/include
        ../src
        ${EDP_PLATFORM_SOURCE_DIR}/src
        ${EDP_SYSTEM_SOURCE_DIR}/src
        ${EDP_MEMORY_SOURCE_DIR}/src
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        espidf_memory_provider_host_tests
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
    )
endif()


add_executable(
    espidf_random_provider_host_tests
    host/RandomByteSourceProviderHostTests.cpp
)

target_include_directories(
    espidf_random_provider_host_tests
    PRIVATE
        host/include
        ../src
        ${EDP_PLATFORM_SOURCE_DIR}/src
        ${EDP_SYSTEM_SOURCE_DIR}/src
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        espidf_random_provider_host_tests
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
    )
endif()
