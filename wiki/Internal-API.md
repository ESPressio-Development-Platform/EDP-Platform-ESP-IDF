# Internal API

Internal adapters map EDP results to ESP-IDF/Mbed TLS outcomes, translate memory requests into heap-capability masks, retain caller-supplied task backing and perform segmented authenticated-data processing without concatenation buffers.

## Deployment-key internal provider contract

`Detail::ProvisionedDeploymentKeySourceZeroizationRequirement` expresses the exact same-domain, exactly-one zeroizer requirement reused by source lifecycle ordering.

`ZeroizationContract` instantiates EDP-Security's `SecretZeroizationTraits` at provider definition time.

The source's SHA/HMAC/HKDF helpers are private implementation, but their invariants define the concrete provider boundary:

- all byte-segment descriptors are validated;
- SHA contexts are freed on every path;
- HMAC pads, PRK and unpublished derived output are erased;
- canonical information is produced only through EDP-Security's encoder;
- caller output is published only after complete success;
- provider failure clears output.

These helpers are not extension points. A replacement root source should implement the generic `DeploymentKeySource` contract independently rather than reuse private functions.
