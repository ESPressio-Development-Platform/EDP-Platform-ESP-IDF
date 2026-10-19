# Resources, Lifecycle and Concurrency

## Execution and memory providers

ExecutionContext uses caller-provided `StaticTask_t`/stack backing and static synchronization. Memory-resource behaviour is determined by ESP-IDF heap capabilities and remains caller-request driven.

## AES-GCM

AES-GCM uses bounded tail/authentication scratch and no plaintext-publication buffer proportional to message length. The selected ByteOperations provider erases scratch.

## Secret zeroizer

`SecretZeroizationProvider` is stateless. It borrows each mutable range only for the call and retains nothing.

## Provisioned deployment-key source

Instance-owned secret bytes are exactly:

~~~text
ApplicationKeyBytes + DeploymentPskBytes
~~~

plus dependency pointer, lifecycle flag and ABI padding.

Derivation stack capacity is fixed by root extents, SHA-256 block/digest widths and `ApplicationContextCapacity`. There is no provider-owned heap allocation or input-dependent storage growth.

`Initialize` copies caller roots. The caller must erase its staging buffers. `Destroy` erases both private copies and is idempotent. The destructor calls `Destroy`. Reprovisioning is permitted only after destruction.

The zeroizer must outlive the source. The source must outlive the generic deployment-key provider which borrows it.

## Concurrency

Operations on one source/key-provider instance are caller-serialized. Neither owns synchronization. Applications requiring cross-context access must serialize at a higher layer or confine ownership.

Provisioning, derivation and key rotation are not ISR operations. Provider-specific ISR support remains explicit for the separate SpinLock operations.


## H429 native atomic qualification

The opt-in native atomic provider requires ESP32 or ESP32-S3, __XTENSA__
and XCHAL_HAVE_S32C1I. All complete Word storage must be internal DRAM,
checked before initialization. ESP32-S2 and external/RTC storage are refused;
there is no emulation, additional storage, task or lock. See
[provider reference](Reference-AtomicWord32-ESP-IDF).

H433: reserve ADC/internal RF/ESP32 I2S while owning SDK entropy, serialize Bootstrap lifecycle and stop consumers before release. One reused inline native4B word plus compiler startup guard metadata; one boolean per provider. No heap/task/mutex/new RNG algorithm.
