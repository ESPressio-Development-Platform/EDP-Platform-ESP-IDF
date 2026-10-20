# Native monotonic timebase example

Arduino IDE, PlatformIO Arduino and PlatformIO ESP-IDF use identical Example.hpp.
Install this native provider plus EDP-Clock, EDP-Platform and EDP-System for the
standalone Arduino IDE; use C++20 and ESP32/ESP32-S3. PlatformIO forms use sibling
checkouts directly and provide both targets. The IDF path needs reviewed SDK5.5.5;
other SDKs may still compile but prerequisite inspection explicitly refuses them.

Bootstrap owns one stateless source and one canonical MonotonicClock. The printed
typed prerequisite code is software configuration inspection only, not measured
physical accuracy or permission to transmit/promote Clock authority. No timer is
allocated or changed. See [the full conditional contract](../../docs/NATIVE-MONOTONIC-TIMEBASE.MD).
No board execution or standalone Arduino IDE run is claimed for H467.
