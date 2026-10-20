# H467 complete native dependency review

Owning native closure: eight packages; application closure: 23 packages. One justified mandatory edge is added: native Platform-ESP-IDF to the existing abstract Clock capability. No application node, optional edge or package version is added. Every edge below is mandatory. Repeated dependencies refer to their complete row. SDK/FreeRTOS are target facilities, not EDP packages.

| Package | Direct dependencies |
|---|---|
| EDP-Mesh | EDP-System, EDP-Persistence, EDP-BoundedTopology, EDP-RemoteSystem, EDP-Primitives, EDP-Radio, EDP-Security, EDP-Threading, EDP-Clock, EDP-Memory, EDP-Serialisation, EDP-Command, EDP-Event, EDP-State, EDP-Platform |
| EDP-System | None |
| EDP-Persistence | EDP-System |
| EDP-BoundedTopology | None |
| EDP-RemoteSystem | EDP-System, EDP-BoundedTopology, EDP-Memory, EDP-Threading |
| EDP-Memory | EDP-System, EDP-Platform, EDP-BoundedTopology |
| EDP-Platform | EDP-System |
| EDP-Threading | EDP-System, EDP-Platform, EDP-Clock, EDP-BoundedTopology, EDP-Memory |
| EDP-Clock | EDP-System, EDP-Platform |
| EDP-Primitives | EDP-System, EDP-Serialisation |
| EDP-Serialisation | EDP-System, EDP-BoundedTypes, EDP-BoundedTopology, EDP-Memory, EDP-Platform-Portable, EDP-Localisation |
| EDP-BoundedTypes | EDP-Memory, EDP-Platform-Portable |
| EDP-Platform-Portable | EDP-Platform, EDP-Memory |
| EDP-Localisation | EDP-System, EDP-Memory |
| EDP-Radio | EDP-System, EDP-Clock, EDP-RemoteSystem |
| EDP-Security | EDP-System, EDP-Platform, EDP-Memory |
| EDP-Command | EDP-System, EDP-Primitives, EDP-Memory, EDP-Threading, EDP-Clock, EDP-Serialisation |
| EDP-Event | EDP-System, EDP-Primitives, EDP-Memory, EDP-Threading, EDP-Clock, EDP-BoundedTopology, EDP-BoundedTypes |
| EDP-State | EDP-Primitives, EDP-System, EDP-BoundedTypes, EDP-Clock, EDP-RemoteSystem, EDP-BoundedTopology, EDP-Memory, EDP-Threading |
| EDP-Platform-ESP-IDF | EDP-Platform, EDP-Memory, EDP-Security, EDP-Persistence, EDP-System, EDP-Clock |
| EDP-Platform-FreeRTOS | EDP-Platform |
| EDP-Persistence-ESP-IDF | EDP-Persistence, EDP-Memory |
| EDP-Radio-NRF24 | EDP-Radio |

Clock points only to abstract Platform/System, never back to the concrete native provider. Bootstrap selects one source/canonical Clock; no alternate timing subsystem is introduced. Hardware specification compliance and calibration are not inferred from this graph.
