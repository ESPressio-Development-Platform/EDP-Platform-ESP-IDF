# Resources, Lifecycle and Concurrency

ExecutionContext uses caller-provided StaticTask_t/stack backing and static synchronization. Memory-resource behaviour is determined by ESP-IDF heap capabilities and remains caller-request driven. AES-GCM uses bounded scratch state and no plaintext-publication buffer proportional to message length. ISR support is explicit per provider operation.
