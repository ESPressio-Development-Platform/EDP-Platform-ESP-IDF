# Composition

Provider types offer capabilities in the existing Platform, Memory and Security domains; the repository does not invent ESP-IDF-specific domains. Requirements are satisfied through compile-time Composition and Bootstrap-owned instances.

The dependency direction remains acyclic because abstract EDP-Security depends on abstract Platform/Memory, while this concrete repository implements Security providers.
