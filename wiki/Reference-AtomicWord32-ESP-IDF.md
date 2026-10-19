# H428 ESP-IDF hardware atomic-word provider

[Details, limitations and test evidence](../docs/ATOMIC-WORD-32.MD). Explicit include only on qualified Xtensa ESP32, 32-bit 4B lock-free CAS via vendor \`esp_cpu_compare_and_set\`, acquire/release MEMW. S3 50k CAS two cores and WROOM cross-build qualified; not a Clock authority certificate or RISC-V implementation.
