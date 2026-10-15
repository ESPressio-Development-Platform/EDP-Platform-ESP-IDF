// H360 target-side inclusion and template-instantiation smoke test
#include <security/EcdsaP256Sha256Provider.hpp>
#include <mbedtls/bignum.h>
#include <ESPressio_Platform.hpp>

struct H360Source {
    const mbedtls_mpi* ResolveSigningScalar(
        ESPressio::Security::DigitalSigningKeyReference) const noexcept {
        return nullptr;
    }
};
struct H360Random {
    ESPressio::Platform::Randomness::RandomByteGenerationResult FillRandomBytes(
        std::uint8_t*, std::size_t) noexcept {
        return ESPressio::Platform::Randomness::RandomByteGenerationResult::ProviderFailure;
    }
};
using H360Provider = ESPressio::Platform::ESPIDF::Security::EcdsaP256Sha256Provider<
    H360Source,H360Random>;
using H360Contract=ESPressio::Security::Detail::DigitalSignatureProviderTraits<H360Provider>;
static_assert(H360Contract::SignatureBytes == 64U);
static_assert(H360Contract::PublicKeyBytes == 65U);
ESPressio::Security::DigitalSignatureSignResult H360Smoke(
    H360Provider& provider,
    ESPressio::Security::ByteView message,
    ESPressio::Security::MutableByteView dest
) { return provider.Sign({2U,3U},message,dest); }
ESPressio::Security::DigitalSignatureVerifyResult H360Check(
    const H360Provider& provider,
    ESPressio::Security::ByteView key,
    ESPressio::Security::ByteView message,
    ESPressio::Security::ByteView sig
) { return provider.Verify(key,message,sig); }
