# AtomicWord32 Provider - ESP-IDF

## Purpose

This library satisfies the `EDP-Platform` capability:

```cpp
ESPressio::Platform::Concurrency::AtomicWord32
```

for the ESP-IDF concrete environment.

The provider type is:

```cpp
ESPressio::Platform::ESPIDF::Concurrency::AtomicWord32Provider
```

## Contract

The provider advertises:

```text
LockFree               = true
AtomicWordStorageBytes = 4
```

and exposes:

```cpp
AtomicWord32Provider::Word
```

with the Platform operations:

```cpp
LoadRelaxed()
LoadAcquire()
StoreRelaxed(...)
StoreRelease(...)
CompareExchangeAcqRel(...)
```

## Concrete mechanism

The implementation uses `std::atomic<std::uint32_t>` supplied by the target C++ toolchain.

This is a deliberate concrete choice rather than an assumption in `EDP-Platform`. The ESP-IDF provider enforces at compile time that the concrete atomic is both:

- always lock-free;
- exactly four bytes.

If either guarantee is false for a selected target/toolchain, this concrete provider fails compilation instead of silently substituting a hidden lock.

## Memory

Each `Word` occupies exactly four bytes and owns no additional state.

There is:

```text
no heap allocation
no mutex
no semaphore
no task
no queue
no observer/callback registry
```

## ConcurrentSnapshot

The provider can be selected directly by the platform-neutral utility:

```cpp
using Snapshot = ESPressio::Platform::Concurrency::ConcurrentSnapshot<
    MyState,
    ESPressio::Platform::ESPIDF::Concurrency::AtomicWord32Provider
>;
```

This keeps the coherent-snapshot algorithm in `EDP-Platform` while the concrete atomic primitive remains owned by the ESP-IDF Platform library.

## SDK boundary

No higher ESPressio domain needs to include ESP-IDF-specific headers or vocabulary merely to use synchronized state publication.

Application Bootstrap selects the concrete provider, and consuming domains depend only on the `EDP-Platform` capability contract.
