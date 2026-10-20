# H467 native canonical timebase source qualification

Platform-ESP-IDF#12. The existing SDK esp_timer_get_time wrapper is now owned by
EDP-Platform-ESP-IDF and implements the existing EDP-Clock MonotonicTimebase
capability. One-byte empty concrete, unchanged positive/zero/negative count
semantics, same canonical conversion. Existing native providers, Clock/Radio/
NRF/Mesh production source and retained state are unchanged. No timer, Clock,
task, lock, heap, wire field, estimator or package version is added. The justified
mandatory native-to-Clock contract edge is acyclic; full eight-owner and23-native
closure with all mandatory/transitive edges is in H467-DEPENDENCY-REVIEW.MD.
The legacy native NVS repository's existing documentation, rather than a missing
package JSON, supplies its reviewed direct edges; no absent metadata is invented.

Typed ordinary-context inspection checks exact reviewed SDK5.5.5, target, build,
debugger, configured40MHz XTAL, ESP32APB80MHz when applicable and positivecounter.
S3 uses XTAL-backed SYSTIMER; ESP32 uses APB-backed LAC. Fixed frequency queries
are configuration-derived, not analog measurements. PrerequisitesMatched supplies
no accuracy, physical compliance, transmit/lease or Root authority token. The
manufacturer-aware conditional contract retains150ppm/25us/1us inputs and requires
normal operation with frozen source/configuration binding, no sleep/debug/counter
changes and actual hardware compliance. This is the user's accepted specification
approach, without inventing calibration or requesting unavailable instruments.
See NATIVE-MONOTONIC-TIMEBASE.MD for exact obligations and primary sources.

Final owning CTest Release passes bothGCC/Clang: four portable native-provider
executables, AtomicWord32 refusal suite and the new timebase suite per compiler.
The actual new provider runs twelve explicit SDK MODEL build variants per compiler;
normal CTest, assertion-enabled Release and ASan/UBSan pass24variants each, and
Clang static analysis passes12. Runtime tests cover debugger/query/frequency/
counter failures, no construction SDK side effects, raw signed-boundary equivalence
and existing canonical conversion. Initial normal receipt precedes the added raw
boundary assertions; final CTest/quality receipts include them. Models measure no
native physical frequency or analog accuracy. No optional genuine MbedTLS suite
rerun or hosted CI success is claimed; unchanged crypto providers pass their
existing controlled host suites in both compiler Release CTest runs.

Four new demo builds (ESP-IDF/Arduino, WROOM/S3) and six maintained compile-only
native compositions (two canonical, four H447 composition) pass with exact source
before/after equality. Three example headers are identical. Current maintained
H436/H447 sources now select the owning provider; historical source capsules,
consumed profiles and H466 actual source remain frozen. Never flash spent images.
Both emitted canonical callback closures pass strict IRAM/code/literal/shared-data
and exact SDK dispatch/source checks; S3 missing/changed SDK binding negatives
still refuse. This proves the inspected callback placement, not full ISR closure,
latency, live counter drift, physical precision or certificate authority.

Raw receipts, commands/states, source capsules, SDK hashes, before/after sources,
wrapper migration, complete graph and placement results are in evidence/H467.
One original graph-harness failure from assuming package JSON for native NVS is
preserved separately; its corrected declared-dependency run passes. This task
makes no board/serial/flash/key/profile/NVS/host-time/RPI400 action. Current private
baseline remains H466 final C49/D66 snapshots, both last confirmed ROM-halted.
All H466 and older profiles are consumed. H46622 quiet-window intervals below1ms
remain conditional observations outside lateNVS; concurrent holdover/exchange,
same-candidate remote+self journal, receiver/lease/quorum/partition/rejoin/fleet/
resource/fullV2 proofs remain open. Root stays disabled.
