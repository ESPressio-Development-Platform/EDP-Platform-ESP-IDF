# Demos

- `atomic-word-basic` — the ESP-IDF `AtomicWord32` concrete provider.

The repository remains the ESP-IDF concrete backend. The policy-required Arduino PlatformIO target disables PlatformIO's framework compatibility filter locally so the current SDK-independent atomic provider surface can be compiled there without changing the package's declared ESP-IDF ownership or compatibility.
