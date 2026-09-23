# Private Implementation

Authenticated decryption must never publish unauthenticated plaintext. The implementation therefore authenticates before caller-visible plaintext publication, using fixed scratch storage and explicit erasure through ByteOperations.

ExternalPreferredMemoryResource tries PSRAM first and only then internal memory. ExecutionContext does not use forced deletion as a normal lifecycle mechanism.
