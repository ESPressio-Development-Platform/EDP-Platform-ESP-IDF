# ESP-IDF ExecutionContext Provider

The provider uses ESP-IDF's static core-affinity task creation API.

The caller supplies both stack storage and native task-control storage.

The provider advertises processor-affinity support and uses byte-based stack sizing as defined by ESP-IDF.

A created task begins behind an internal start gate. When the user entry function returns, the provider signals completion and suspends the native task.

`Join()` is the ownership synchronization point after a started entry returns. `Destroy()` refuses to forcibly delete a running or unjoined execution scope.
