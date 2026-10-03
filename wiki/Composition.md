# Composition

Provider Types offer capabilities in the existing Platform, Memory and Security domains; this repository defines no ESP-IDF-specific domain.

## Security providers

| Provider | Offer | Important properties | Requirements |
|---|---|---|---|
| `Aes256GcmProvider<TByteOperationsProvider>` | shared `AuthenticatedCrypto` | AES-256-GCM, 32-byte key, 12-byte default nonce, 16-byte tag, RawMaterial | exactly one external-domain ByteOperations |
| `SecretZeroizationProvider` | shared `SecretZeroization` | none | none |
| `ProvisionedDeploymentKeySourceProvider<...>` | exclusive `DeploymentKeySource` | HKDF-SHA-256, application-key-as-salt ordering, 32-byte output, fixed context capacity | exactly one same-domain SecretZeroization |

The provisioned source declares `InitializesAfter` and `ShutsDownBefore` for its selected zeroizer. These constraints make the borrowed zeroizer lifetime explicit to Bootstrap.

EDP-Security's generic `DeploymentKeyProvider<Source, Zeroizer, Capacity>` independently requires the exact source properties and one same-domain zeroizer. Its configured context capacity must not exceed the source capacity.

## Selection and substitution

Provider selection is compile-time. Concrete provider Types are passed directly to application topology/Bootstrap and validated through EDP trait contracts.

A future opaque-root source substitutes at the `DeploymentKeySource` capability boundary. It must advertise the same Suite V1 properties to qualify for the existing generic deployment-key provider.

## Other domains

Execution, synchronization, randomness and memory providers retain their established offers and requirements. The new security facilities add no cycle: abstract EDP-Security does not depend on this concrete repository.
