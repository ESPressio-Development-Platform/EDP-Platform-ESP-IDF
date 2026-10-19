# ESP-IDF explicit random-byte source example

H433 migration: all three forms explicitly acquire the internal SDK entropy source while ADC/internal RF/ESP32 I2S consumers are absent. Destruction releases it. Default nonzero requests now refuse. Follow the complete [resource contract](../../docs/EXPLICIT-ENTROPY-OWNERSHIP.MD); do not reuse this setup alongside those conflicting peripherals.
