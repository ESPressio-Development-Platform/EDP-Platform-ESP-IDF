cmake_minimum_required(VERSION 3.16)

project(EDPPlatformESPIDFMemoryHostTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Normalize caller-supplied sibling paths relative to the repository root.
if(DEFINED EDP_PLATFORM_SOURCE_DIR)
    get_filename_component(
        EDP_PLATFORM_SOURCE_DIR
        "${EDP_PLATFORM_SOURCE_DIR}"
        ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.."
    )
endif()

if(DEFINED EDP_SYSTEM_SOURCE_DIR)
    get_filename_component(
        EDP_SYSTEM_SOURCE_DIR
        "${EDP_SYSTEM_SOURCE_DIR}"
        ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.."
    )
endif()

if(DEFINED EDP_MEMORY_SOURCE_DIR)
    get_filename_component(
        EDP_MEMORY_SOURCE_DIR
        "${EDP_MEMORY_SOURCE_DIR}"
        ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.."
    )
endif()

if(DEFINED EDP_SECURITY_SOURCE_DIR)
    get_filename_component(
        EDP_SECURITY_SOURCE_DIR
        "${EDP_SECURITY_SOURCE_DIR}"
        ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.."
    )
endif()



if(NOT DEFINED EDP_PLATFORM_SOURCE_DIR)
    message(FATAL_ERROR "EDP_PLATFORM_SOURCE_DIR must point at an EDP-Platform checkout")
endif()

if(NOT DEFINED EDP_SYSTEM_SOURCE_DIR)
    message(FATAL_ERROR "EDP_SYSTEM_SOURCE_DIR must point at an EDP-System checkout")
endif()

if(NOT DEFINED EDP_MEMORY_SOURCE_DIR)
    message(FATAL_ERROR "EDP_MEMORY_SOURCE_DIR must point at an EDP-Memory checkout")
endif()

if(NOT DEFINED EDP_SECURITY_SOURCE_DIR)
    message(FATAL_ERROR "EDP_SECURITY_SOURCE_DIR must point at an EDP-Security checkout")
endif()


# Required dependency headers must exist in the supplied source checkouts.
if(NOT EXISTS "${EDP_SYSTEM_SOURCE_DIR}/src/ESPressio_System.hpp")
    message(FATAL_ERROR "EDP_SYSTEM_SOURCE_DIR does not contain src/ESPressio_System.hpp; pull/update EDP-System")
endif()

if(NOT EXISTS "${EDP_PLATFORM_SOURCE_DIR}/src/ESPressio_Platform.hpp")
    message(FATAL_ERROR "EDP_PLATFORM_SOURCE_DIR does not contain src/ESPressio_Platform.hpp; pull/update EDP-Platform")
endif()

if(NOT EXISTS "${EDP_MEMORY_SOURCE_DIR}/src/ESPressio_Memory.hpp")
    message(FATAL_ERROR "EDP_MEMORY_SOURCE_DIR does not contain src/ESPressio_Memory.hpp; pull/update EDP-Memory")
endif()

if(NOT EXISTS "${EDP_SECURITY_SOURCE_DIR}/src/ESPressio_Security.hpp")
    message(FATAL_ERROR "EDP_SECURITY_SOURCE_DIR does not contain src/ESPressio_Security.hpp; pull/update EDP-Security")
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


add_executable(
    espidf_aes256_gcm_provider_host_tests
    host/Aes256GcmProviderHostTests.cpp
)

target_include_directories(
    espidf_aes256_gcm_provider_host_tests
    PRIVATE
        host/include
        ../src
        ${EDP_PLATFORM_SOURCE_DIR}/src
        ${EDP_SYSTEM_SOURCE_DIR}/src
        ${EDP_MEMORY_SOURCE_DIR}/src
        ${EDP_SECURITY_SOURCE_DIR}/src
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        espidf_aes256_gcm_provider_host_tests
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
    )
endif()
