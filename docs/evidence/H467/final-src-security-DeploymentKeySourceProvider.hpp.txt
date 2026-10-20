#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#if defined(ESP_PLATFORM) && !defined(MBEDTLS_CONFIG_FILE)
#define MBEDTLS_CONFIG_FILE "mbedtls/esp_config.h"
#endif

#include <mbedtls/sha256.h>

#include <ESPressio_Security.hpp>

namespace ESPressio::Platform::ESPIDF::Security {

    namespace Framework = ESPressio::System::CompositionFramework;

    /// Outcome from provisioning the two deployment root secrets.
    enum class ProvisionedDeploymentKeySourceInitializationResult : std::uint8_t {
        Succeeded = 0,
        AlreadyInitialized = 1,
        InvalidApplicationKey = 2,
        InvalidDeploymentPsk = 3
    };


    namespace Detail {

        /// Exact secure-zeroization dependency of a provisioned deployment-key source.
        using ProvisionedDeploymentKeySourceZeroizationRequirement = Framework::Requirement<
            ESPressio::Security::SecretZeroization,
            Framework::RequirementScope::SameDomain,
            Framework::ExactlyProviders<1U>
        >;

    } // ESPressio::Platform::ESPIDF::Security::Detail


    /// Fixed-capacity provisioned root-secret source for Mesh V1 deployment working keys.
    ///
    /// The provider copies an application-wide key and an operator deployment PSK into private,
    /// bounded storage at Bootstrap. Derive performs HKDF-SHA-256 with the application key as salt,
    /// the deployment PSK as input key material and the canonical EDP-Security information field.
    /// Neither root secret is exposed after provisioning. Calls on one instance are caller-serialized.
    template<
        class TZeroizer,
        std::size_t TApplicationKeyBytes,
        std::size_t TDeploymentPskBytes,
        std::size_t TApplicationContextCapacity
    >
    class ProvisionedDeploymentKeySourceProvider final : public Framework::Provider<
        ESPressio::Security::Domain,
        Framework::Offers<
            Framework::Offer<
                ESPressio::Security::DeploymentKeySource,
                Framework::PropertyValue<
                    ESPressio::Security::DeploymentKeySourceAlgorithm,
                    ESPressio::Security::DeploymentKeyDerivationAlgorithm::HkdfSha256
                >,
                Framework::PropertyValue<
                    ESPressio::Security::DeploymentKeySourceInputOrder,
                    ESPressio::Security::DeploymentKeyInputOrder::ApplicationKeyAsSaltDeploymentPskAsInputKeyMaterial
                >,
                Framework::PropertyValue<
                    ESPressio::Security::DeploymentKeySourceOutputBytes,
                    ESPressio::Security::DeploymentWorkingKeyBytes
                >,
                Framework::PropertyValue<
                    ESPressio::Security::DeploymentKeySourceApplicationContextCapacity,
                    TApplicationContextCapacity
                >
            >
        >,
        Framework::Contract<
            Detail::ProvisionedDeploymentKeySourceZeroizationRequirement,
            Framework::InitializesAfter<
                Detail::ProvisionedDeploymentKeySourceZeroizationRequirement
            >,
            Framework::ShutsDownBefore<
                Detail::ProvisionedDeploymentKeySourceZeroizationRequirement
            >
        >
    > {

        private:

            using ZeroizationContract = ESPressio::Security::Detail::SecretZeroizationTraits<
                TZeroizer
            >;

            static_assert(
                std::is_same_v<typename ZeroizationContract::ZeroizeResult, void>,
                "Selected zeroizer must satisfy SecretZeroizationTraits"
            );
            static_assert(
                TApplicationKeyBytes > 0U,
                "Provisioned application key must contain at least one byte"
            );
            static_assert(
                TDeploymentPskBytes > 0U,
                "Provisioned deployment PSK must contain at least one byte"
            );
            static_assert(
                TApplicationContextCapacity > 0U && TApplicationContextCapacity <= 0xFFFFU,
                "Deployment-key application-context capacity must be between 1 and 65535 bytes"
            );

            static constexpr std::size_t Sha256BlockBytes = 64U;
            static constexpr std::size_t Sha256DigestBytes = 32U;

            static_assert(
                ESPressio::Security::DeploymentWorkingKeyBytes == Sha256DigestBytes,
                "Mesh V1 deployment working keys require one SHA-256 digest"
            );

            TZeroizer* _zeroizer;
            std::array<std::uint8_t, TApplicationKeyBytes> _applicationKey{};
            std::array<std::uint8_t, TDeploymentPskBytes> _deploymentPsk{};
            bool _initialized = false;


            template<std::size_t TBytes>
            void Zeroize(
                std::uint8_t (&bytes)[TBytes]
            ) noexcept {
                _zeroizer->SecureZeroize({bytes, TBytes});
            }


            template<std::size_t TBytes>
            void Zeroize(
                std::array<std::uint8_t, TBytes>& bytes
            ) noexcept {
                _zeroizer->SecureZeroize({bytes.data(), bytes.size()});
            }


            static void CopyBytes(
                std::uint8_t* destination,
                const std::uint8_t* source,
                std::size_t size
            ) noexcept {
                for (std::size_t index = 0U; index < size; ++index) {
                    destination[index] = source[index];
                }
            }


            bool Sha256(
                const ESPressio::Security::ByteView* segments,
                std::size_t segmentCount,
                std::uint8_t* digest
            ) noexcept {
                mbedtls_sha256_context context;
                mbedtls_sha256_init(&context);

                int result = mbedtls_sha256_starts(
                    &context,
                    0
                );

                for (
                    std::size_t index = 0U;
                    result == 0 && index < segmentCount;
                    ++index
                ) {
                    const auto& segment = segments[index];

                    if (!segment.IsValid()) {
                        result = -1;
                    } else if (!segment.IsEmpty()) {
                        result = mbedtls_sha256_update(
                            &context,
                            segment.Data,
                            segment.Size
                        );
                    }
                }

                if (result == 0) {
                    result = mbedtls_sha256_finish(
                        &context,
                        digest
                    );
                }

                mbedtls_sha256_free(&context);

                if (result != 0) {
                    _zeroizer->SecureZeroize({digest, Sha256DigestBytes});
                    return false;
                }

                return true;
            }


            bool HmacSha256(
                ESPressio::Security::ByteView key,
                const ESPressio::Security::ByteView* segments,
                std::size_t segmentCount,
                std::uint8_t* output
            ) noexcept {
                std::uint8_t keyBlock[Sha256BlockBytes]{};
                std::uint8_t innerDigest[Sha256DigestBytes]{};
                bool succeeded = true;

                if (key.Size > Sha256BlockBytes) {
                    const ESPressio::Security::ByteView keySegment[1U]{key};
                    succeeded = Sha256(
                        keySegment,
                        1U,
                        keyBlock
                    );
                } else {
                    CopyBytes(
                        keyBlock,
                        key.Data,
                        key.Size
                    );
                }

                if (succeeded) {
                    for (auto& value : keyBlock) value ^= 0x36U;

                    mbedtls_sha256_context context;
                    mbedtls_sha256_init(&context);

                    int result = mbedtls_sha256_starts(
                        &context,
                        0
                    );

                    if (result == 0) {
                        result = mbedtls_sha256_update(
                            &context,
                            keyBlock,
                            sizeof(keyBlock)
                        );
                    }

                    for (
                        std::size_t index = 0U;
                        result == 0 && index < segmentCount;
                        ++index
                    ) {
                        const auto& segment = segments[index];

                        if (!segment.IsValid()) {
                            result = -1;
                        } else if (!segment.IsEmpty()) {
                            result = mbedtls_sha256_update(
                                &context,
                                segment.Data,
                                segment.Size
                            );
                        }
                    }

                    if (result == 0) {
                        result = mbedtls_sha256_finish(
                            &context,
                            innerDigest
                        );
                    }

                    mbedtls_sha256_free(&context);
                    succeeded = result == 0;
                }

                if (succeeded) {
                    for (auto& value : keyBlock) {
                        value ^= static_cast<std::uint8_t>(0x36U ^ 0x5CU);
                    }

                    const ESPressio::Security::ByteView outerSegments[2U]{
                        {keyBlock, sizeof(keyBlock)},
                        {innerDigest, sizeof(innerDigest)}
                    };

                    succeeded = Sha256(
                        outerSegments,
                        2U,
                        output
                    );
                }

                if (!succeeded) {
                    _zeroizer->SecureZeroize({output, Sha256DigestBytes});
                }

                Zeroize(innerDigest);
                Zeroize(keyBlock);
                return succeeded;
            }


            bool DeriveHkdfSha256(
                ESPressio::Security::ByteView info,
                std::uint8_t* output
            ) noexcept {
                std::uint8_t pseudorandomKey[Sha256DigestBytes]{};
                std::uint8_t derivedKey[Sha256DigestBytes]{};

                const ESPressio::Security::ByteView extractSegments[1U]{
                    {_deploymentPsk.data(), _deploymentPsk.size()}
                };

                bool succeeded = HmacSha256(
                    {_applicationKey.data(), _applicationKey.size()},
                    extractSegments,
                    1U,
                    pseudorandomKey
                );

                const std::uint8_t firstBlockIndex[1U]{0x01U};
                const ESPressio::Security::ByteView expandSegments[2U]{
                    info,
                    {firstBlockIndex, sizeof(firstBlockIndex)}
                };

                if (succeeded) {
                    succeeded = HmacSha256(
                        {pseudorandomKey, sizeof(pseudorandomKey)},
                        expandSegments,
                        2U,
                        derivedKey
                    );
                }

                if (succeeded) {
                    CopyBytes(
                        output,
                        derivedKey,
                        sizeof(derivedKey)
                    );
                } else {
                    _zeroizer->SecureZeroize({output, Sha256DigestBytes});
                }

                Zeroize(derivedKey);
                Zeroize(pseudorandomKey);
                return succeeded;
            }

        public:

            static constexpr std::size_t ApplicationKeyBytes = TApplicationKeyBytes;
            static constexpr std::size_t DeploymentPskBytes = TDeploymentPskBytes;
            static constexpr std::size_t ApplicationContextCapacity = TApplicationContextCapacity;
            static constexpr std::size_t OwnedRootSecretBytes =
                ApplicationKeyBytes + DeploymentPskBytes;
            static constexpr std::size_t MaximumDerivationInfoBytes =
                ESPressio::Security::DeploymentKeyDerivationInfoFixedBytes +
                ApplicationContextCapacity;


            explicit ProvisionedDeploymentKeySourceProvider(
                TZeroizer& zeroizer
            ) noexcept :
                _zeroizer(&zeroizer) {}


            ~ProvisionedDeploymentKeySourceProvider() noexcept {
                Destroy();
            }

            ProvisionedDeploymentKeySourceProvider(
                const ProvisionedDeploymentKeySourceProvider&
            ) = delete;
            ProvisionedDeploymentKeySourceProvider& operator =(
                const ProvisionedDeploymentKeySourceProvider&
            ) = delete;
            ProvisionedDeploymentKeySourceProvider(
                ProvisionedDeploymentKeySourceProvider&&
            ) = delete;
            ProvisionedDeploymentKeySourceProvider& operator =(
                ProvisionedDeploymentKeySourceProvider&&
            ) = delete;


            ProvisionedDeploymentKeySourceInitializationResult Initialize(
                ESPressio::Security::ByteView applicationKey,
                ESPressio::Security::ByteView deploymentPsk
            ) noexcept {
                if (_initialized) {
                    return ProvisionedDeploymentKeySourceInitializationResult::AlreadyInitialized;
                }

                if (
                    !applicationKey.IsValid() ||
                    applicationKey.Size != ApplicationKeyBytes
                ) {
                    return ProvisionedDeploymentKeySourceInitializationResult::InvalidApplicationKey;
                }

                if (
                    !deploymentPsk.IsValid() ||
                    deploymentPsk.Size != DeploymentPskBytes
                ) {
                    return ProvisionedDeploymentKeySourceInitializationResult::InvalidDeploymentPsk;
                }

                CopyBytes(
                    _applicationKey.data(),
                    applicationKey.Data,
                    applicationKey.Size
                );
                CopyBytes(
                    _deploymentPsk.data(),
                    deploymentPsk.Data,
                    deploymentPsk.Size
                );

                _initialized = true;
                return ProvisionedDeploymentKeySourceInitializationResult::Succeeded;
            }


            void Destroy() noexcept {
                if (!_initialized) return;

                Zeroize(_deploymentPsk);
                Zeroize(_applicationKey);
                _initialized = false;
            }


            ESPressio::Security::DeploymentKeyDerivationResult Derive(
                const ESPressio::Security::DeploymentKeyDerivationRequest& request,
                ESPressio::Security::MutableByteView output
            ) noexcept {
                if (
                    !request.IsValid() ||
                    request.ApplicationContext.Size > ApplicationContextCapacity
                ) {
                    return ESPressio::Security::DeploymentKeyDerivationResult::InvalidRequest;
                }

                if (
                    !output.IsValid() ||
                    output.Size != ESPressio::Security::DeploymentWorkingKeyBytes
                ) {
                    return ESPressio::Security::DeploymentKeyDerivationResult::InvalidOutput;
                }

                if (!_initialized) {
                    return ESPressio::Security::DeploymentKeyDerivationResult::RootSecretUnavailable;
                }

                std::array<std::uint8_t, MaximumDerivationInfoBytes> info{};
                std::size_t writtenBytes = 0U;

                const auto writeResult = ESPressio::Security::WriteDeploymentKeyDerivationInfo(
                    request,
                    {info.data(), info.size()},
                    writtenBytes
                );

                if (
                    writeResult != ESPressio::Security::DeploymentKeyDerivationInfoResult::Succeeded
                ) {
                    Zeroize(info);
                    return ESPressio::Security::DeploymentKeyDerivationResult::ProviderFailure;
                }

                const bool succeeded = DeriveHkdfSha256(
                    {info.data(), writtenBytes},
                    output.Data
                );

                Zeroize(info);
                return succeeded
                    ? ESPressio::Security::DeploymentKeyDerivationResult::Succeeded
                    : ESPressio::Security::DeploymentKeyDerivationResult::ProviderFailure;
            }

    };

} // ESPressio::Platform::ESPIDF::Security
