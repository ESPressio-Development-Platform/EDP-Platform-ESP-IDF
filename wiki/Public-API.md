# Public API

Concrete provider types implement the abstract Platform, Memory and Security capabilities.

ExecutionContext uses static ESP-IDF task creation and supports processor affinity. SpinLock wraps `portMUX_TYPE` and can expose interrupt-safe operations where the native API supports them. RandomByteSource uses `esp_fill_random` and advertises cryptographic suitability.

Memory resources expose internal-only, external/PSRAM-only and external-preferred allocation policies. AES-256-GCM advertises the exact algorithm, 32-byte key, 12-byte data-protection nonce, 16-byte tag and RawMaterial key access mode.

Exact declarations remain authoritative in the exported headers.
