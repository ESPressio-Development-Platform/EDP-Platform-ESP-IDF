# randomness/RandomByteSourceProvider.hpp — H433

**Classification:** public native provider and ownership statuses; private state.
Existing capability/typed Fill contract and version0.1.0 remain. See the
[complete resource preconditions and evidence](../docs/EXPLICIT-ENTROPY-OWNERSHIP.MD).

## Public declarations

InternalEntropyAcquisitionResult : uint8_t is Acquired=0, AlreadyAcquired=1,
OwnedByAnotherInstance=2, UnsupportedTarget=3. InternalEntropyReleaseResult is
Released=0, NotOwned=1. None of these values is a caller-provided trust input.

RandomByteSourceProvider inherits the existing Framework Provider in Platform
Domain and offers RandomByteSource with CryptographicallySuitable metadata.
That metadata applies to successful nonzero output under the resource contract;
it is not a ready predicate. Default construction is side-effect-free with
respect to entropy peripherals. Copy/move construction and assignment are deleted.
Destructor noexcept releases only this instance's actually acquired source.

- AcquireInternalEntropySource() noexcept returns the acquisition enum. Bootstrap
  reserves all SDK conflicts first. Supported hardware claims the shared native
  word atomically and enables the SDK source exactly once; a competitor returns
  OwnedByAnotherInstance, self reentry AlreadyAcquired. Unsupported hardware refuses.
- ReleaseInternalEntropySource() noexcept returns the release enum; SDK disable
  precedes publication of available global ownership. Nonowners return NotOwned.
- OwnsInternalEntropySource() const noexcept is an observation of this instance's
  actual ownership, not a min-entropy measurement or permission to violate lifetime.
- FillRandomBytes(uint8_t* destination,size_t byteCount) noexcept returns the
  original RandomByteGenerationResult. Zero bytes succeed without access. Null
  nonzero destination refuses. Held, supported source calls esp_fill_random;
  otherwise ProviderFailure with no weak fallback or implicit acquisition.
- RandomByteSourceContract aliases the existing generic provider traits.

## Private declarations

_ownsInternalEntropy is one boolean, set only after actual SDK enable and
cleared after actual disable. _sourceOwner is an inline static instance of the
existing native AtomicWord32Provider::Word, one across TUs; only the audited
Xtensa ESP32/S3 branch declares it. Native word storage qualification remains
mandatory. No second atomic/RNG implementation or caller-supplied readiness flag.

## Conditional compilation

sdkconfig.h supplies SDK-owned CONFIG_IDF_TARGET_ESP32 / ESP32S3 and compiler
__XTENSA__. Those select the only audited positive source path and the existing
native atomic header. Other targets compile explicit refusal. No repository-owned
macro bypasses entropy qualification. Host sdkconfig selects no target; positive
native instructions and SDK enable/disable are exercised on actual C+D hardware.
