// H360 real Mbed TLS ECDSA P-256 verification with host-only ephemeral test key.
// NEVER use this demo key or random state on an ESP32 or in production.
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/random.h>
#include <cerrno>
#include <mbedtls/ecp.h>
#include <mbedtls/bignum.h>

#include "../src/security/EcdsaP256Sha256Provider.hpp"

namespace {
  namespace Security = ESPressio::Security;
  struct TestKeys final {
    mbedtls_mpi secret{};
    TestKeys(){mbedtls_mpi_init(&secret);assert(mbedtls_mpi_lset(&secret,1)==0);}
    ~TestKeys(){mbedtls_mpi_free(&secret);}
    const mbedtls_mpi* ResolveSigningScalar(Security::DigitalSigningKeyReference r) const noexcept {
      return r.Identifier==1U && r.Generation==1U ? &secret : nullptr;
    }
  };
  // Test-only Linux cryptographic entropy source. Production uses the
  // Bootstrap-selected EDP-Platform ESP-IDF RandomByteSource capability.
  struct TestEntropy final {
    ESPressio::Platform::Randomness::RandomByteGenerationResult FillRandomBytes(
      std::uint8_t* data,std::size_t count) noexcept {
      return getrandom(data,count,0)==static_cast<ssize_t>(count) ?
        ESPressio::Platform::Randomness::RandomByteGenerationResult::Succeeded :
        ESPressio::Platform::Randomness::RandomByteGenerationResult::ProviderFailure;
    }
  };
}

int main() {
  using Provider=ESPressio::Platform::ESPIDF::Security::
    EcdsaP256Sha256Provider<TestKeys,TestEntropy>;
  using Traits=Security::Detail::DigitalSignatureProviderTraits<Provider>;
  static_assert(Traits::SignatureBytes==64U && Traits::PublicKeyBytes==65U);
  TestKeys keys;
  TestEntropy random;
  Provider provider(keys,random);
  // SEC1 uncompressed P-256 base point; private key 1 is TEST-ONLY.
  constexpr char publicHex[] =
    "046B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0"
    "F4A13945D898C2964FE342E2FE1A7F9B8EE7EB4A7C0F9E16"
    "2BCE33576B315ECECBB6406837BF51F5";
  auto nibble=[](char c)->std::uint8_t{
      if(c>='0'&&c<='9')return c-'0';
      if(c>='A'&&c<='F')return c-'A'+10;
      return c-'a'+10;
  };
  std::uint8_t publicKey[65U]{};
  static_assert(sizeof(publicHex)-1U==130U);
  for(std::size_t i=0;i<65U;++i)
      publicKey[i]=static_cast<std::uint8_t>(
          (nibble(publicHex[i*2U])<<4U)|nibble(publicHex[i*2U+1U]));
  const std::uint8_t msg[]="EDP.H360.TEST-ELECTORATE-B.GRANT.v1";
  std::uint8_t signature[64U]{};
  assert(provider.Sign({1U,1U},{msg,sizeof(msg)},{signature,sizeof(signature)})==
    Security::DigitalSignatureSignResult::Succeeded);
  assert(provider.Verify({publicKey,65U},{msg,sizeof(msg)},{signature,64U})==
    Security::DigitalSignatureVerifyResult::Verified);
  std::uint8_t badMessage[sizeof(msg)]{};
  std::memcpy(badMessage,msg,sizeof(msg));
  badMessage[10]^=1U;
  assert(provider.Verify({publicKey,65U},{badMessage,sizeof(badMessage)},{signature,64U})==
    Security::DigitalSignatureVerifyResult::InvalidSignature);
  std::uint8_t tampered[64U]{};
  std::memcpy(tampered,signature,64U);
  tampered[31]^=1U;
  assert(provider.Verify({publicKey,65U},{msg,sizeof(msg)},{tampered,64U})==
    Security::DigitalSignatureVerifyResult::InvalidSignature);
  assert(provider.Sign({2U,1U},{msg,sizeof(msg)},{signature,64U})==
    Security::DigitalSignatureSignResult::KeyUnavailable);

  // A mathematically valid ECDSA high-S partner MUST be rejected on wire.
  constexpr std::uint8_t orderBytes[32U]={
    0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xbc,0xe6,0xfa,0xad,0xa7,0x17,0x9e,0x84,
    0xf3,0xb9,0xca,0xc2,0xfc,0x63,0x25,0x51};
  mbedtls_mpi order,s,hi;
  mbedtls_mpi_init(&order);mbedtls_mpi_init(&s);mbedtls_mpi_init(&hi);
  assert(mbedtls_mpi_read_binary(&order,orderBytes,32U)==0);
  assert(mbedtls_mpi_read_binary(&s,signature+32U,32U)==0);
  assert(mbedtls_mpi_sub_mpi(&hi,&order,&s)==0);
  std::uint8_t highS[64U]{};
  std::memcpy(highS,signature,64U);
  assert(mbedtls_mpi_write_binary(&hi,highS+32U,32U)==0);
  assert(provider.Verify({publicKey,65U},{msg,sizeof(msg)},{highS,64U})==
    Security::DigitalSignatureVerifyResult::InvalidSignature);
  mbedtls_mpi_free(&hi);mbedtls_mpi_free(&s);mbedtls_mpi_free(&order);

  // Distinct valid P-256 public key (private scalar 2), not same voter.
  constexpr char otherHex[]=
    "047CF27B188D034F7E8A52380304B51AC3C08969E277F21B35"
    "A60B48FC4766997807775510DB8ED040293D9AC69F7430DBB"
    "A7DADE63CE982299E04B79D227873D1";
  static_assert(sizeof(otherHex)-1U==130U);
  std::uint8_t wrongVoter[65U]{};
  for(std::size_t i=0;i<65U;++i)
    wrongVoter[i]=static_cast<std::uint8_t>(
      (nibble(otherHex[i*2U])<<4U)|nibble(otherHex[i*2U+1U]));
  assert(provider.Verify({wrongVoter,65U},{msg,sizeof(msg)},{signature,64U})==
    Security::DigitalSignatureVerifyResult::InvalidSignature);
  assert(provider.Verify({publicKey,64U},{msg,sizeof(msg)},{signature,64U})==
    Security::DigitalSignatureVerifyResult::InvalidInput);
  assert(provider.Verify({publicKey,65U},{msg,sizeof(msg)},{signature,63U})==
    Security::DigitalSignatureVerifyResult::InvalidInput);
  assert(provider.Verify({publicKey,65U},{msg,sizeof(msg)},{highS,64U})==
    Security::DigitalSignatureVerifyResult::InvalidSignature);

  std::puts("H360_REAL_MBEDTLS_P256_SHA256_SIGN_VERIFY_WRONG_VOTER_HIGH_S_PASS_NO_ELECTION");
}
