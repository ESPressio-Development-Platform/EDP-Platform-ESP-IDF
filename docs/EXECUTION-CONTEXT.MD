# ESP-IDF ExecutionContext Provider

The provider uses ESP-IDF's static core-affinity task creation API.

The caller supplies both stack storage and native task-control storage.

The provider advertises processor-affinity support and uses byte-based stack sizing as defined by ESP-IDF.

A created task begins behind an internal start gate. When the user entry function returns, the provider publishes completion and suspends the native task until the owner calls `Destroy()`.

`Destroy()` refuses to forcibly delete a still-running user execution scope.
