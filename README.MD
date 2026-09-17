# ESPressio Platform - ESP-IDF

`EDP-Platform-ESP-IDF` supplies ESP-IDF-environment concrete implementations for abstractions defined by `EDP-Platform`.

## Contents

- [AtomicWord32 Provider](./docs/ATOMIC-WORD32.MD)  
  Supplies the lock-free four-byte `AtomicWord32` capability required by Platform read-mostly concurrency facilities such as `ConcurrentSnapshot`.

## Dependencies

This library depends on `EDP-Platform` and the C++ standard atomic facility available in the ESP-IDF target toolchain.

The current provider does not allocate memory, create tasks, use a mutex/semaphore, or depend on a scheduler. Compilation deliberately fails on a target where `std::atomic<uint32_t>` is not always lock-free or does not occupy four bytes.

## Development Status

Only Platform capabilities with an actual ESP-IDF concrete responsibility are added here. The current implementation is intentionally limited to the atomic-word prerequisite required by synchronized Clock work.
