#pragma once

// H360 genuine ECDSA P-256 SHA-256 provider, ESP-IDF/Mbed TLS concrete layer.
// Signed grants are NOT elected authority until caller proves durable journal,
// lease/Era fencing and independently attributable fixed-electorate quorum.

#include <cstddef>
#include <cstdint>
#include <type_traits>

#if defined(ESP_PLATFORM) && !defined(MBEDTLS_CONFIG_FILE)
#define MBEDTLS_CONFIG_FILE "mbedtls/esp_config.h"
#endif

#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
#include <mbedtls/bignum.h>
#include <mbedtls/sha256.h>

#include <ESPressio_Security.hpp>
#include <ESPressio_Platform.hpp>

namespace ESPressio::Platform::ESPIDF::Security {

    namespace Framework = ESPressio::System::CompositionFramework;

    // TKeySource must provide:
    //   const mbedtls_mpi* ResolveSigningScalar(
    //       ESPressio::Security::DigitalSigningKeyReference) const noexcept
    // Keys remain owned by the separately provisioned Bootstrap key store.
    // TRandomSource must provide EDP-Platform FillRandomBytes() noexcept.
    template<class TKeySource, class TRandomSource>
    class EcdsaP256Sha256Provider final : public Framework::Provider<
        ESPressio::Security::Domain,
        Framework::Offers<
            Framework::Offer<
                ESPressio::Security::DigitalSignature,
                Framework::PropertyValue<
                    ESPressio::Security::DigitalSignatureAlgorithm,
                    ESPressio::Security::SignatureAlgorithm::EcdsaP256Sha256
                >,
                Framework::PropertyValue<
                    ESPressio::Security::DigitalSignaturePublicKeyBytes, 65U
                >,
                Framework::PropertyValue<
                    ESPressio::Security::DigitalSignatureSignatureBytes, 64U
                >
            >
        >
    > {
    private:
        const TKeySource& _keys;
        TRandomSource& _rng;

        static int Random(void* context, unsigned char* output, size_t count) {
            auto* source = static_cast<TRandomSource*>(context);
            return source->FillRandomBytes(output,count) ==
                ESPressio::Platform::Randomness::RandomByteGenerationResult::Succeeded
                ? 0 : -1;
        }

        static bool ValidInputs(
            ESPressio::Security::ByteView message
        ) noexcept {
            return message.IsValid() && !message.IsEmpty();
        }

        // P-256 order in canonical big-endian bytes. Mbed TLS 3.6 can hide
        // the group order behind its backend and exposes no group.N field.
        static int LoadP256Order(mbedtls_mpi& order) noexcept {
            static constexpr std::uint8_t bytes[32U] = {
                0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                0xbc,0xe6,0xfa,0xad,0xa7,0x17,0x9e,0x84,
                0xf3,0xb9,0xca,0xc2,0xfc,0x63,0x25,0x51
            };
            return mbedtls_mpi_read_binary(&order,bytes,sizeof(bytes));
        }

        static bool LowS(
            const mbedtls_mpi& order, const mbedtls_mpi& s
        ) noexcept {
            mbedtls_mpi half;
            mbedtls_mpi_init(&half);
            const int copied = mbedtls_mpi_copy(&half,&order);
            if (copied == 0) {
                mbedtls_mpi_shift_r(&half,1U);
            }
            const bool ok = copied == 0 &&
                mbedtls_mpi_cmp_int(&s,1) >= 0 &&
                mbedtls_mpi_cmp_mpi(&s,&half) <= 0;
            mbedtls_mpi_free(&half);
            return ok;
        }

    public:
        EcdsaP256Sha256Provider(
            const TKeySource& keys, TRandomSource& random
        ) noexcept : _keys(keys),_rng(random) {}

        ESPressio::Security::DigitalSignatureSignResult Sign(
            ESPressio::Security::DigitalSigningKeyReference reference,
            ESPressio::Security::ByteView message,
            ESPressio::Security::MutableByteView signature
        ) noexcept {
            using Result = ESPressio::Security::DigitalSignatureSignResult;
            if (!reference.IsValid() || !ValidInputs(message) ||
                !signature.IsValid()) return Result::InvalidInput;
            if (signature.Size < 64U) return Result::OutputCapacityUnavailable;

            const mbedtls_mpi* secret = _keys.ResolveSigningScalar(reference);
            if (secret==nullptr) return Result::KeyUnavailable;

            unsigned char hash[32U]{};
            if (mbedtls_sha256(message.Data,message.Size,hash,0)!=0)
                return Result::ProviderFailure;

            mbedtls_ecp_group grp; mbedtls_ecp_group_init(&grp);
            mbedtls_mpi r,s,order; mbedtls_mpi_init(&r); mbedtls_mpi_init(&s); mbedtls_mpi_init(&order);
            int rc=mbedtls_ecp_group_load(&grp,MBEDTLS_ECP_DP_SECP256R1);
            if(rc==0)rc=LoadP256Order(order);
            if (rc==0 && (mbedtls_mpi_cmp_int(secret,1)<0 ||
                mbedtls_mpi_cmp_mpi(secret,&order)>=0))
                rc=-1;
            if (rc==0) rc=mbedtls_ecdsa_sign(
                &grp,&r,&s,secret,hash,sizeof(hash),Random,&_rng
            );
            // Convert high-S signature to canonical low-S. Both are valid
            // mathematical ECDSA but only low-S belongs to wire contract.
            if (rc==0 && !LowS(order,s))
                rc=mbedtls_mpi_sub_mpi(
                    &s,&order,&s
                );
            if (rc==0) rc=mbedtls_mpi_write_binary(&r,signature.Data,32U);
            if (rc==0) rc=mbedtls_mpi_write_binary(&s,signature.Data+32U,32U);
            const bool ok=rc==0 && LowS(order,s) &&
                mbedtls_mpi_cmp_int(&r,1)>=0 &&
                mbedtls_mpi_cmp_mpi(&r,&order)<0;
            if (!ok) {
                for(std::size_t i=0U;i<64U;++i)signature.Data[i]=0U;
            }
            mbedtls_mpi_free(&order);mbedtls_mpi_free(&s);mbedtls_mpi_free(&r);
            mbedtls_ecp_group_free(&grp);
            for(auto& b:hash)b=0U;
            return ok ? Result::Succeeded : Result::ProviderFailure;
        }

        ESPressio::Security::DigitalSignatureVerifyResult Verify(
            ESPressio::Security::ByteView publicKey,
            ESPressio::Security::ByteView message,
            ESPressio::Security::ByteView signature
        ) const noexcept {
            using Result = ESPressio::Security::DigitalSignatureVerifyResult;
            if(!publicKey.IsValid() || publicKey.Size!=65U ||
                publicKey.Data[0]!=0x04U ||
                !ValidInputs(message) ||
                !signature.IsValid() || signature.Size!=64U)
                return Result::InvalidInput;
            unsigned char hash[32U]{};
            if(mbedtls_sha256(message.Data,message.Size,hash,0)!=0)
                return Result::ProviderFailure;
            mbedtls_ecp_group grp; mbedtls_ecp_group_init(&grp);
            mbedtls_ecp_point q; mbedtls_ecp_point_init(&q);
            mbedtls_mpi r,s,order;mbedtls_mpi_init(&r);mbedtls_mpi_init(&s);mbedtls_mpi_init(&order);
            int rc=mbedtls_ecp_group_load(&grp,MBEDTLS_ECP_DP_SECP256R1);
            if(rc==0)rc=LoadP256Order(order);
            if(rc==0)rc=mbedtls_ecp_point_read_binary(
                &grp,&q,publicKey.Data,publicKey.Size
            );
            if(rc==0)rc=mbedtls_ecp_check_pubkey(&grp,&q);
            if(rc==0)rc=mbedtls_mpi_read_binary(&r,signature.Data,32U);
            if(rc==0)rc=mbedtls_mpi_read_binary(&s,signature.Data+32U,32U);
            if(rc==0 && (mbedtls_mpi_cmp_int(&r,1)<0 ||
                mbedtls_mpi_cmp_mpi(&r,&order)>=0 ||
                !LowS(order,s)))rc=-1;
            if(rc==0)rc=mbedtls_ecdsa_verify(
                &grp,hash,sizeof(hash),&q,&r,&s
            );
            mbedtls_mpi_free(&order);mbedtls_mpi_free(&s);mbedtls_mpi_free(&r);
            mbedtls_ecp_point_free(&q);mbedtls_ecp_group_free(&grp);
            for(auto& b:hash)b=0U;
            return rc==0 ? Result::Verified : Result::InvalidSignature;
        }
    };

} // namespace ESPressio::Platform::ESPIDF::Security
