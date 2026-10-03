#include <Arduino.h>

#include <array>
#include <cstddef>
#include <cstdint>

#include <ESPressio_Platform_ESP_IDF.hpp>
#include <ESPressio_Security.hpp>

namespace Demo {

    using Zeroizer = ESPressio::Platform::ESPIDF::Security::SecretZeroizationProvider;
    using Source = ESPressio::Platform::ESPIDF::Security::ProvisionedDeploymentKeySourceProvider<
        Zeroizer,
        32U,
        32U,
        16U
    >;
    using Keys = ESPressio::Security::DeploymentKeyProvider<
        Source,
        Zeroizer,
        16U
    >;


    /// Demonstrates protected root provisioning and the complete bounded deployment-key lifecycle.
    int Run() noexcept {
        Zeroizer zeroizer;
        Source source(zeroizer);

        std::array<std::uint8_t, 32U> applicationKey{};
        std::array<std::uint8_t, 32U> deploymentPsk{};

        for (std::size_t index = 0U; index < applicationKey.size(); ++index) {
            applicationKey[index] = static_cast<std::uint8_t>(0x10U + index);
            deploymentPsk[index] = static_cast<std::uint8_t>(0x80U + index);
        }

        const auto sourceResult = source.Initialize(
            {applicationKey.data(), applicationKey.size()},
            {deploymentPsk.data(), deploymentPsk.size()}
        );

        zeroizer.SecureZeroize({
            deploymentPsk.data(),
            deploymentPsk.size()
        });
        zeroizer.SecureZeroize({
            applicationKey.data(),
            applicationKey.size()
        });

        if (
            sourceResult !=
            ESPressio::Platform::ESPIDF::Security::ProvisionedDeploymentKeySourceInitializationResult::Succeeded
        ) {
            return 1;
        }

        Keys keys(source, zeroizer);
        const std::uint8_t applicationContext[]{
            'd', 'e', 'm', 'o', '.', 'm', 'e', 's', 'h', '.', 'v', '1'
        };

        if (
            keys.Initialize({
                1U,
                {applicationContext, sizeof(applicationContext)},
                ESPressio::Security::KeyGeneration(1U)
            }) != ESPressio::Security::DeploymentKeyProviderInitializationResult::Succeeded
        ) {
            return 2;
        }

        ESPressio::Security::KeySelection hopSelection;
        if (
            keys.ResolveCurrent(
                ESPressio::Security::DeploymentKeyPurpose::HopProtection,
                hopSelection
            ) != ESPressio::Security::KeyResolutionResult::Succeeded
        ) {
            return 3;
        }

        ESPressio::Security::RawKeyMaterialView hopMaterial;
        if (
            keys.AccessRawKey(
                hopSelection,
                hopMaterial
            ) != ESPressio::Security::KeyMaterialAccessResult::Succeeded ||
            hopMaterial.Size != ESPressio::Security::DeploymentWorkingKeyBytes
        ) {
            return 4;
        }

        if (
            keys.Rotate(
                ESPressio::Security::KeyGeneration(2U)
            ) != ESPressio::Security::DeploymentKeyRotationResult::Succeeded
        ) {
            return 5;
        }

        ESPressio::Security::KeyGenerationState previousState =
            ESPressio::Security::KeyGenerationState::Retired;

        if (
            keys.GetGenerationState(
                hopSelection.Id,
                ESPressio::Security::KeyGeneration(1U),
                previousState
            ) != ESPressio::Security::KeyGenerationStateResult::Succeeded ||
            previousState != ESPressio::Security::KeyGenerationState::Retained
        ) {
            return 6;
        }

        if (
            keys.RetireDeploymentGeneration(
                ESPressio::Security::KeyGeneration(1U)
            ) != ESPressio::Security::DeploymentKeySetRetirementResult::Retired
        ) {
            return 7;
        }

        keys.Destroy();
        source.Destroy();
        return keys.IsInitialized() ? 8 : 0;
    }

} // Demo


void setup() {
    static_cast<void>(
        Demo::Run()
    );
}

void loop() {}
