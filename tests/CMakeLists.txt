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

if(DEFINED EDP_BOUNDED_TOPOLOGY_SOURCE_DIR)
    get_filename_component(
        EDP_BOUNDED_TOPOLOGY_SOURCE_DIR
        "${EDP_BOUNDED_TOPOLOGY_SOURCE_DIR}"
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

if(NOT DEFINED EDP_BOUNDED_TOPOLOGY_SOURCE_DIR)
    message(FATAL_ERROR "EDP_BOUNDED_TOPOLOGY_SOURCE_DIR must point at an EDP-BoundedTopology checkout")
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

if(NOT EXISTS "${EDP_BOUNDED_TOPOLOGY_SOURCE_DIR}/src/ESPressio_BoundedTopology.hpp")
    message(FATAL_ERROR "EDP_BOUNDED_TOPOLOGY_SOURCE_DIR does not contain src/ESPressio_BoundedTopology.hpp; pull/update EDP-BoundedTopology")
endif()

find_package(OpenSSL REQUIRED)

# H360 optional genuine Mbed TLS digital-signature verification.
# Unlike standard host scaffolding, this target requires a real Mbed TLS 3.6
# header/library pair. It must NEVER use the fake AES-GCM host headers.
option(EDP_ENABLE_REAL_P256_MBEDTLS_TESTS
    "Enable genuine ECDSA P-256 signature host validation" OFF)
if(EDP_ENABLE_REAL_P256_MBEDTLS_TESTS)
    if(NOT MBEDTLS_INCLUDE_DIR OR NOT EXISTS "${MBEDTLS_INCLUDE_DIR}/mbedtls/ecdsa.h")
        message(FATAL_ERROR "Specify matching real MBEDTLS_INCLUDE_DIR")
    endif()
    if(NOT MBEDCRYPTO_LIBRARY OR NOT EXISTS "${MBEDCRYPTO_LIBRARY}")
        message(FATAL_ERROR "Specify actual linked MBEDCRYPTO_LIBRARY")
    endif()
    add_executable(
        espidf_real_p256_signature_host_tests
        host/EcdsaP256Sha256ProviderHostTests.cpp
    )
    target_include_directories(espidf_real_p256_signature_host_tests PRIVATE
        ${MBEDTLS_INCLUDE_DIR}
        ../src
        ${EDP_SECURITY_SOURCE_DIR}/src
        ${EDP_SYSTEM_SOURCE_DIR}/src
        ${EDP_PLATFORM_SOURCE_DIR}/src
        ${EDP_MEMORY_SOURCE_DIR}/src
        ${EDP_BOUNDED_TOPOLOGY_SOURCE_DIR}/src
    )
    target_link_libraries(espidf_real_p256_signature_host_tests
        PRIVATE ${MBEDCRYPTO_LIBRARY})
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(espidf_real_p256_signature_host_tests
            PRIVATE -Wall -Wextra -Wpedantic -Werror -UNDEBUG)
    endif()
    enable_testing()
    add_test(NAME espidf_real_p256_signature_host_tests
        COMMAND espidf_real_p256_signature_host_tests)
endif()

# H363: optional real Mbed TLS development-only P-256 identity key lifecycle
# over the EDP-Persistence KeyValueStorage contract; fake storage host only.
# Does not provision voters or permit Clock election.
if(EDP_ENABLE_REAL_P256_MBEDTLS_TESTS AND DEFINED EDP_PERSISTENCE_SOURCE_DIR)
    add_executable(espidf_lab_p256_key_source_host_tests
        host/DevelopmentP256KeySourceHostTests.cpp)
    target_include_directories(espidf_lab_p256_key_source_host_tests PRIVATE
        ${MBEDTLS_INCLUDE_DIR}
        ../src
        ${EDP_PERSISTENCE_SOURCE_DIR}/src
        ${EDP_SECURITY_SOURCE_DIR}/src
        ${EDP_SYSTEM_SOURCE_DIR}/src
        ${EDP_PLATFORM_SOURCE_DIR}/src
        ${EDP_MEMORY_SOURCE_DIR}/src
        ${EDP_BOUNDED_TOPOLOGY_SOURCE_DIR}/src
    )
    target_link_libraries(espidf_lab_p256_key_source_host_tests
        PRIVATE ${MBEDCRYPTO_LIBRARY})
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(espidf_lab_p256_key_source_host_tests PRIVATE
            -Wall -Wextra -Wpedantic -Werror -UNDEBUG)
    endif()
    enable_testing()
    add_test(NAME espidf_lab_p256_key_source_host_tests
        COMMAND espidf_lab_p256_key_source_host_tests)
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
        ${EDP_BOUNDED_TOPOLOGY_SOURCE_DIR}/src
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
        ${EDP_BOUNDED_TOPOLOGY_SOURCE_DIR}/src
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


add_executable(
    espidf_deployment_key_source_provider_host_tests
    host/DeploymentKeySourceProviderHostTests.cpp
)

target_include_directories(
    espidf_deployment_key_source_provider_host_tests
    PRIVATE
        host/include
        ../src
        ${EDP_PLATFORM_SOURCE_DIR}/src
        ${EDP_SYSTEM_SOURCE_DIR}/src
        ${EDP_MEMORY_SOURCE_DIR}/src
        ${EDP_BOUNDED_TOPOLOGY_SOURCE_DIR}/src
        ${EDP_SECURITY_SOURCE_DIR}/src
)

target_link_libraries(
    espidf_deployment_key_source_provider_host_tests
    PRIVATE
        OpenSSL::Crypto
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        espidf_deployment_key_source_provider_host_tests
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
    )
endif()
