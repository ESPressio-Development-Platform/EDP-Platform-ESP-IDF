# Architecture

## Role

This repository is a concrete-provider boundary. It translates abstract ESPressio contracts into ESP-IDF facilities without moving platform policy into the abstract repositories.

| Domain | Concrete facilities |
|---|---|
| EDP-Platform | static execution context, `portMUX_TYPE` spin lock, hardware/system random bytes |
| EDP-Memory | internal RAM, external/PSRAM-only, external-preferred/internal-fallback resources |
| EDP-Security | AES-256-GCM, secure zeroization, provisioned deployment-key source |

## Security split

EDP-Security owns algorithm identities, request/result Types, canonical deployment-key information, six purpose domains, generations, rotation, retention and retirement.

EDP-Platform-ESP-IDF owns:

- the Mbed TLS AES-GCM call boundary;
- the Mbed TLS secure-zeroization primitive;
- loading/copying caller-provisioned application and deployment roots into bounded RAM;
- the allocation-free Mbed TLS SHA-256 orchestration which satisfies the generic HKDF source contract.

The generic `DeploymentKeyProvider` never receives either root. It asks the selected source to derive one purpose-labelled generation directly into private working-key storage.

## Root boundary

The initial concrete source is deliberately a provisioned RAM source. Application Bootstrap supplies an application-wide symmetric key and an operator deployment PSK as configuration/provisioning data. The source copies exact template-selected extents and exposes no root accessor.

A later eFuse, secure-element or opaque-handle source may replace it by offering the same EDP-Security capability and properties. No generic key lifecycle code may assume RAM roots or this concrete class.

## Determinism

All new provider instance storage and derivation scratch are compile-time bounded. The source avoids generic `mbedtls_hkdf`/message-digest setup because that route may allocate HMAC context storage. It uses Mbed TLS SHA-256 streaming operations with fixed HMAC/HKDF orchestration.

## Ownership boundaries

- Bootstrap owns provider objects and establishes dependency lifetimes.
- Callers own input/output buffers.
- The source owns only its fixed root copies.
- The generic deployment-key provider owns derived working-key generations.
- Neither security provider owns a task, queue, worker or lock.

API ownership remains explicit: portable primitives stay portable, generic security remains generic and only ESP-IDF-specific integration lives here.
