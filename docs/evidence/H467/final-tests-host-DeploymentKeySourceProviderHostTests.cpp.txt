#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <mbedtls/platform_util.h>
#include <mbedtls/sha256.h>

#include <security/DeploymentKeySourceProvider.hpp>
#include <security/SecretZeroizationProvider.hpp>

namespace Test {

    using Zeroizer = ESPressio::Platform::ESPIDF::Security::SecretZeroizationProvider;
    using Source = ESPressio::Platform::ESPIDF::Security::ProvisionedDeploymentKeySourceProvider<
        Zeroizer,
        32U,
        32U,
        16U
    >;
    using InitializationResult =
        ESPressio::Platform::ESPIDF::Security::ProvisionedDeploymentKeySourceInitializationResult;
    using DerivationResult = ESPressio::Security::DeploymentKeyDerivationResult;
    using Purpose = ESPressio::Security::DeploymentKeyPurpose;

    using ZeroizationContract = ESPressio::Security::Detail::SecretZeroizationTraits<Zeroizer>;
    using SourceContract = ESPressio::Security::Detail::DeploymentKeySourceTraits<Source>;

    static_assert(std::is_same_v<typename ZeroizationContract::ZeroizeResult, void>);
    static_assert(
        SourceContract::Algorithm ==
        ESPressio::Security::DeploymentKeyDerivationAlgorithm::HkdfSha256
    );
    static_assert(
        SourceContract::InputOrder ==
        ESPressio::Security::DeploymentKeyInputOrder::ApplicationKeyAsSaltDeploymentPskAsInputKeyMaterial
    );
    static_assert(SourceContract::OutputBytes == 32U);
    static_assert(SourceContract::ApplicationContextCapacity == 16U);
    static_assert(Source::ApplicationKeyBytes == 32U);
    static_assert(Source::DeploymentPskBytes == 32U);
    static_assert(Source::ApplicationContextCapacity == 16U);
    static_assert(Source::OwnedRootSecretBytes == 64U);
    static_assert(
        Source::MaximumDerivationInfoBytes ==
        ESPressio::Security::DeploymentKeyDerivationInfoFixedBytes + 16U
    );
    static_assert(!std::is_copy_constructible_v<Source>);
    static_assert(!std::is_copy_assignable_v<Source>);
    static_assert(!std::is_move_constructible_v<Source>);
    static_assert(!std::is_move_assignable_v<Source>);


    constexpr std::array<std::uint8_t, 32U> ApplicationKey{
        0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U,
        0x08U, 0x09U, 0x0AU, 0x0BU, 0x0CU, 0x0DU, 0x0EU, 0x0FU,
        0x10U, 0x11U, 0x12U, 0x13U, 0x14U, 0x15U, 0x16U, 0x17U,
        0x18U, 0x19U, 0x1AU, 0x1BU, 0x1CU, 0x1DU, 0x1EU, 0x1FU
    };

    constexpr std::array<std::uint8_t, 32U> DeploymentPsk{
        0x20U, 0x21U, 0x22U, 0x23U, 0x24U, 0x25U, 0x26U, 0x27U,
        0x28U, 0x29U, 0x2AU, 0x2BU, 0x2CU, 0x2DU, 0x2EU, 0x2FU,
        0x30U, 0x31U, 0x32U, 0x33U, 0x34U, 0x35U, 0x36U, 0x37U,
        0x38U, 0x39U, 0x3AU, 0x3BU, 0x3CU, 0x3DU, 0x3EU, 0x3FU
    };

    constexpr std::uint8_t ApplicationContext[]{
        'E', 'D', 'P', '-', 'M', 'e', 's', 'h', '-', 'K', 'A', 'T', '-', 'v', '1'
    };

    constexpr std::array<std::array<std::uint8_t, 32U>, 6U> ExpectedKeys{{
        {{
            0x81U, 0x86U, 0x04U, 0x78U, 0xE7U, 0x68U, 0x0FU, 0x4DU,
            0xF5U, 0x30U, 0x17U, 0xA2U, 0x91U, 0x4AU, 0x7FU, 0x8BU,
            0xAAU, 0xC4U, 0x96U, 0xE0U, 0xB4U, 0xB5U, 0x3EU, 0x4EU,
            0xCBU, 0x45U, 0xFBU, 0x43U, 0x63U, 0xC8U, 0xEDU, 0x41U
        }},
        {{
            0xB9U, 0xA3U, 0x17U, 0x81U, 0x20U, 0x28U, 0x78U, 0x97U,
            0xD0U, 0x4AU, 0xEDU, 0xF8U, 0x17U, 0xD3U, 0x45U, 0x79U,
            0x9CU, 0x2EU, 0xB2U, 0xB4U, 0x19U, 0xF2U, 0xE2U, 0x79U,
            0x14U, 0xF5U, 0x93U, 0xB7U, 0x43U, 0xE2U, 0x81U, 0x33U
        }},
        {{
            0x15U, 0x79U, 0x7AU, 0xFEU, 0xA4U, 0xA5U, 0xDFU, 0xB4U,
            0x33U, 0x93U, 0x41U, 0x40U, 0x97U, 0xD9U, 0x99U, 0x1BU,
            0xFBU, 0x94U, 0x65U, 0x7BU, 0x72U, 0xEDU, 0x54U, 0x8EU,
            0xD7U, 0x9FU, 0x8FU, 0x3AU, 0x76U, 0x29U, 0x7CU, 0x56U
        }},
        {{
            0xEFU, 0xB9U, 0xB7U, 0xC1U, 0xC3U, 0x63U, 0x04U, 0xD8U,
            0x86U, 0x88U, 0xF1U, 0xFCU, 0xF8U, 0x3BU, 0x9EU, 0xF4U,
            0xC9U, 0x2EU, 0xB9U, 0x04U, 0x92U, 0x42U, 0x01U, 0x9FU,
            0xA1U, 0x04U, 0x21U, 0x24U, 0xC2U, 0xF4U, 0xFBU, 0x98U
        }},
        {{
            0x60U, 0x0DU, 0x36U, 0xB3U, 0xA3U, 0x0AU, 0xA1U, 0x34U,
            0xACU, 0xB2U, 0x97U, 0x9DU, 0x29U, 0xE3U, 0x6DU, 0x33U,
            0x86U, 0x92U, 0xB7U, 0xE8U, 0xABU, 0x80U, 0x66U, 0xC9U,
            0xBCU, 0x4AU, 0x9FU, 0x50U, 0x63U, 0x93U, 0xFFU, 0x37U
        }},
        {{
            0x4FU, 0xBDU, 0x91U, 0x9CU, 0x38U, 0x0EU, 0x4AU, 0xBFU,
            0x0BU, 0x4FU, 0x8FU, 0x36U, 0x6CU, 0x48U, 0x6DU, 0x9FU,
            0x58U, 0x43U, 0xA5U, 0x31U, 0xEDU, 0x56U, 0x95U, 0x33U,
            0xB1U, 0x5BU, 0xDFU, 0x31U, 0x7DU, 0x3FU, 0x64U, 0x32U
        }}
    }};


    template<std::size_t TBytes>
    bool Equal(
        const std::array<std::uint8_t, TBytes>& left,
        const std::array<std::uint8_t, TBytes>& right
    ) noexcept {
        for (std::size_t index = 0U; index < TBytes; ++index) {
            if (left[index] != right[index]) return false;
        }

        return true;
    }


    template<std::size_t TBytes>
    bool AllEqual(
        const std::array<std::uint8_t, TBytes>& values,
        std::uint8_t expected
    ) noexcept {
        for (const auto value : values) {
            if (value != expected) return false;
        }

        return true;
    }


    ESPressio::Security::DeploymentKeyDerivationRequest Request(
        Purpose purpose,
        ESPressio::Security::ByteView applicationContext = {
            ApplicationContext,
            sizeof(ApplicationContext)
        },
        std::uint32_t generation = 0x01020304U
    ) noexcept {
        return {
            ESPressio::Security::DeploymentKeyDerivationVersion::V1,
            ESPressio::Security::DeploymentSecuritySuite::MeshV1,
            0x0102U,
            applicationContext,
            purpose,
            ESPressio::Security::DeploymentKeyDirection::Bidirectional,
            ESPressio::Security::DeploymentKeyIdentifier(purpose),
            ESPressio::Security::KeyGeneration(generation)
        };
    }


    void TestZeroizationProvider() noexcept {
        mbedtls_test_reset_zeroize_evidence();
        Zeroizer zeroizer;
        std::array<std::uint8_t, 8U> secret{
            1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U
        };

        zeroizer.SecureZeroize({nullptr, 0U});
        zeroizer.SecureZeroize({nullptr, 1U});
        assert(mbedtls_test_zeroize_call_count == 0U);

        zeroizer.SecureZeroize({secret.data(), secret.size()});
        assert(mbedtls_test_zeroize_call_count == 1U);
        assert(mbedtls_test_zeroize_byte_count == secret.size());
        assert(AllEqual(secret, 0U));
    }


    void TestProvisioningValidation(
        Source& source
    ) noexcept {
        assert(
            source.Initialize(
                {nullptr, ApplicationKey.size()},
                {DeploymentPsk.data(), DeploymentPsk.size()}
            ) == InitializationResult::InvalidApplicationKey
        );

        assert(
            source.Initialize(
                {ApplicationKey.data(), ApplicationKey.size() - 1U},
                {DeploymentPsk.data(), DeploymentPsk.size()}
            ) == InitializationResult::InvalidApplicationKey
        );

        assert(
            source.Initialize(
                {ApplicationKey.data(), ApplicationKey.size()},
                {DeploymentPsk.data(), DeploymentPsk.size() - 1U}
            ) == InitializationResult::InvalidDeploymentPsk
        );
    }


    void TestProvisionedSource() noexcept {
        mbedtls_test_reset_zeroize_evidence();
        Zeroizer zeroizer;
        Source source(zeroizer);
        std::array<std::uint8_t, 32U> output{};

        output.fill(0xA5U);
        const auto discoveryRequest = Request(Purpose::DiscoveryAdmission);
        assert(
            source.Derive(
                discoveryRequest,
                {output.data(), output.size()}
            ) == DerivationResult::RootSecretUnavailable
        );
        assert(AllEqual(output, 0xA5U));

        TestProvisioningValidation(source);

        assert(
            source.Initialize(
                {ApplicationKey.data(), ApplicationKey.size()},
                {DeploymentPsk.data(), DeploymentPsk.size()}
            ) == InitializationResult::Succeeded
        );
        assert(
            source.Initialize(
                {ApplicationKey.data(), ApplicationKey.size()},
                {DeploymentPsk.data(), DeploymentPsk.size()}
            ) == InitializationResult::AlreadyInitialized
        );

        const ESPressio::Security::DeploymentKeyDerivationRequest invalidRequest{};
        assert(
            source.Derive(
                invalidRequest,
                {output.data(), output.size()}
            ) == DerivationResult::InvalidRequest
        );

        std::uint8_t oversizedContext[17U]{};
        assert(
            source.Derive(
                Request(
                    Purpose::DiscoveryAdmission,
                    {oversizedContext, sizeof(oversizedContext)}
                ),
                {output.data(), output.size()}
            ) == DerivationResult::InvalidRequest
        );

        assert(
            source.Derive(
                discoveryRequest,
                {output.data(), output.size() - 1U}
            ) == DerivationResult::InvalidOutput
        );

        std::array<std::array<std::uint8_t, 32U>, 6U> actualKeys{};
        for (std::size_t index = 0U; index < actualKeys.size(); ++index) {
            const auto purpose = static_cast<Purpose>(index);
            assert(
                source.Derive(
                    Request(purpose),
                    {actualKeys[index].data(), actualKeys[index].size()}
                ) == DerivationResult::Succeeded
            );
            assert(Equal(actualKeys[index], ExpectedKeys[index]));

            if (index > 0U) {
                assert(!Equal(actualKeys[0U], actualKeys[index]));
            }
        }

        const auto baseline = actualKeys[0U];
        std::array<std::uint8_t, 32U> changedGeneration{};
        assert(
            source.Derive(
                Request(
                    Purpose::DiscoveryAdmission,
                    {ApplicationContext, sizeof(ApplicationContext)},
                    0x01020305U
                ),
                {changedGeneration.data(), changedGeneration.size()}
            ) == DerivationResult::Succeeded
        );
        assert(!Equal(baseline, changedGeneration));

        std::uint8_t changedContext[sizeof(ApplicationContext)]{};
        for (std::size_t index = 0U; index < sizeof(changedContext); ++index) {
            changedContext[index] = ApplicationContext[index];
        }
        changedContext[sizeof(changedContext) - 1U] ^= 0x01U;

        std::array<std::uint8_t, 32U> changedContextKey{};
        assert(
            source.Derive(
                Request(
                    Purpose::DiscoveryAdmission,
                    {changedContext, sizeof(changedContext)}
                ),
                {changedContextKey.data(), changedContextKey.size()}
            ) == DerivationResult::Succeeded
        );
        assert(!Equal(baseline, changedContextKey));

        output.fill(0xA5U);
        mbedtls_test_sha256_fail_next_operation = true;
        assert(
            source.Derive(
                discoveryRequest,
                {output.data(), output.size()}
            ) == DerivationResult::ProviderFailure
        );
        assert(!mbedtls_test_sha256_fail_next_operation);
        assert(AllEqual(output, 0U));

        const auto bytesBeforeDestroy = mbedtls_test_zeroize_byte_count;
        source.Destroy();
        assert(
            mbedtls_test_zeroize_byte_count ==
            bytesBeforeDestroy + Source::OwnedRootSecretBytes
        );

        output.fill(0x5AU);
        assert(
            source.Derive(
                discoveryRequest,
                {output.data(), output.size()}
            ) == DerivationResult::RootSecretUnavailable
        );
        assert(AllEqual(output, 0x5AU));

        const auto bytesBeforeIdempotentDestroy = mbedtls_test_zeroize_byte_count;
        source.Destroy();
        assert(mbedtls_test_zeroize_byte_count == bytesBeforeIdempotentDestroy);

        assert(
            source.Initialize(
                {DeploymentPsk.data(), DeploymentPsk.size()},
                {ApplicationKey.data(), ApplicationKey.size()}
            ) == InitializationResult::Succeeded
        );

        std::array<std::uint8_t, 32U> reversedRoots{};
        assert(
            source.Derive(
                discoveryRequest,
                {reversedRoots.data(), reversedRoots.size()}
            ) == DerivationResult::Succeeded
        );
        assert(!Equal(baseline, reversedRoots));
        source.Destroy();

        assert(
            source.Initialize(
                {ApplicationKey.data(), ApplicationKey.size()},
                {DeploymentPsk.data(), DeploymentPsk.size()}
            ) == InitializationResult::Succeeded
        );
    }


    void TestDestructorZeroization() noexcept {
        mbedtls_test_reset_zeroize_evidence();
        Zeroizer zeroizer;
        std::size_t bytesBeforeDestruction = 0U;

        {
            Source source(zeroizer);
            assert(
                source.Initialize(
                    {ApplicationKey.data(), ApplicationKey.size()},
                    {DeploymentPsk.data(), DeploymentPsk.size()}
                ) == InitializationResult::Succeeded
            );
            bytesBeforeDestruction = mbedtls_test_zeroize_byte_count;
        }

        assert(
            mbedtls_test_zeroize_byte_count ==
            bytesBeforeDestruction + Source::OwnedRootSecretBytes
        );
    }


    int Run() noexcept {
        TestZeroizationProvider();
        TestProvisionedSource();
        TestDestructorZeroization();
        return 0;
    }

} // Test


int main() {
    return Test::Run();
}
