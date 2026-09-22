# AES-256-GCM Security Provider

## Purpose

`Aes256GcmProvider<TByteOperationsProvider>` implements the EDP-Security `AuthenticatedCrypto` capability using ESP-IDF's Mbed TLS GCM implementation.

It is owned by `EDP-Platform-ESP-IDF` because its implementation directly consumes the ESP-IDF-provided Mbed TLS API.

## Advertised contract

The provider advertises:

- algorithm: `Aes256Gcm`;
- key size: 32 bytes;
- generic Data Protection nonce size: 12 bytes;
- tag size: 16 bytes;
- key access mode: `RawMaterial`.

The provider itself does not choose keys, key generations, Security purposes, Transport sequence values, or Data Protection nonce policy.

## Segmented associated data

The Security contract represents associated data as ordered non-owning segments.

The provider starts one GCM operation and feeds each non-empty segment with `mbedtls_gcm_update_ad`. No temporary AAD concatenation buffer is allocated.

## Caller-owned memory

Ciphertext and plaintext are emitted into caller-owned buffers.

A small fixed stack scratch region is used only for the final partial AES block because the Mbed TLS streaming API permits buffered final output.

The provider binds a Bootstrap-selected EDP-Memory ByteOperations provider and uses that abstraction for tail copies. It performs no heap allocation itself.

## Authentication-before-publication

Decrypt performs two bounded GCM passes:

1. authenticate the ciphertext and calculate the expected tag while discarding candidate plaintext into fixed stack scratch;
2. only after the supplied tag matches, run the transform again and publish plaintext into caller storage.

This deliberately avoids exposing unauthenticated candidate plaintext to the caller.

The tag comparison has no data-dependent early exit.

## Configuration requirement

The target ESP-IDF configuration must include AES and GCM support. ESP-IDF's normal Mbed TLS configuration enables GCM on supported targets.

## Mbed TLS ownership

Mbed TLS error values remain implementation details. The provider maps them to EDP-Security's stable authenticated-encryption/decryption result vocabulary.
