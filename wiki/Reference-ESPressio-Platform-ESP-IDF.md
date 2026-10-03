# src/ESPressio_Platform_ESP_IDF.hpp

**Primary classification:** PUBLIC ENTRY POINT

**Source baseline:** `524a881b9ed27a86c5b886429558242d88962b70`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Platform-ESP-IDF/blob/524a881b9ed27a86c5b886429558242d88962b70/src/ESPressio_Platform_ESP_IDF.hpp)

## Direct includes

- `execution/ExecutionContextProvider.hpp`
- `memory/MemoryResourceProviders.hpp`
- `randomness/RandomByteSourceProvider.hpp`
- `security/Aes256GcmProvider.hpp`
- `security/DeploymentKeySourceProvider.hpp`
- `security/SecretZeroizationProvider.hpp`
- `synchronization/SpinLockProvider.hpp`

## Documented declarations

This header is an aggregation/include surface and contains no declaration-level Doxygen blocks.


The broad umbrella now imports execution, memory, randomness, authenticated-crypto, deployment-key-source, secret-zeroization and synchronization provider families. It remains appropriate for consumers selecting multiple ESP-IDF provider families. Consumers needing only SpinLock should prefer the separate narrow entry point documented at [Reference-ESPressio-Platform-ESP-IDF-SpinLock](Reference-ESPressio-Platform-ESP-IDF-SpinLock).
