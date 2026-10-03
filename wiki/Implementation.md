# Private Implementation

## Authenticated decryption

Unauthenticated plaintext must never become caller-visible. AES-GCM decryption therefore performs an authentication-only pass using fixed scratch, compares tags without data-dependent early exit, then performs a second pass which publishes plaintext only after authentication.

## Deployment HKDF

The provisioned source implements standard HKDF-SHA-256 orchestration over Mbed TLS's low-level SHA-256 streaming API.

Extract uses application key as HMAC key/salt and deployment PSK as message/input key material. Expand hashes the exact canonical EDP-Security information bytes followed by `0x01`.

A single 64-byte HMAC key block is transformed from inner pad to outer pad in place. This reduces transient storage while retaining standard HMAC semantics. Root sizes longer than one SHA-256 block are normalized by hashing.

The generic `mbedtls_hkdf` route is intentionally avoided because its message-digest setup may allocate HMAC context storage. This choice preserves bounded deterministic memory without implementing a replacement hash primitive.

## Failure preservation

The derived key is first produced in local unpublished storage. Cryptographic failure erases both local state and caller output. Validation/root-unavailable failures occur before output mutation.

## Memory resources and execution

ExternalPreferredMemoryResource tries PSRAM before internal memory. ExecutionContext does not use forced deletion as a normal lifecycle mechanism.

No deployment-key provider helper stores global mutable state, starts a task, waits or acquires a lock. One instance is caller-serialized.
