// H362 genuine P-256 with a memory-only fake of EDP-Persistence.
 // Test never claims NVS, RF, election or hardened private-key storage.
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <sys/random.h>
#include <ESPressio_Security.hpp>
#include <ESPressio_Persistence.hpp>
#include <security/DevelopmentP256KeySource.hpp>
#include <security/EcdsaP256Sha256Provider.hpp>

namespace PS=ESPressio::Persistence;
namespace ES=ESPressio::Security;
namespace CF=ESPressio::System::CompositionFramework;
namespace EP=ESPressio::Platform;
using Result=EP::ESPIDF::Security::LabP256KeyResult;
using ID=ESPressio::System::Identity::DeviceIdentifier;

struct MemoryStore final : CF::Provider<PS::Domain,
    CF::Offers<CF::Offer<PS::KeyValueStorage,
      CF::PropertyValue<PS::KeyValueAccessMode,PS::AccessMode::ReadWrite>,
      CF::PropertyValue<PS::KeyValueRetention,PS::RetentionLevel::PowerLoss>,
      CF::PropertyValue<PS::KeyCaseSensitivity,PS::TextCaseSensitivity::CaseSensitive>,
      CF::PropertyValue<PS::KeyValueMediaRemovability,PS::MediaRemovability::Fixed>,
      CF::PropertyValue<PS::MaximumKeyBytes,std::size_t{15U}>,
      CF::PropertyValue<PS::MaximumKeyValueSize,PS::StorageSize{512U}>,
      CF::PropertyValue<PS::KeyEnumerationSupport,PS::Support::Unsupported>,
      CF::PropertyValue<PS::ReadValueAtSupport,PS::Support::Unsupported>,
      CF::PropertyValue<PS::ClearAllSupport,PS::Support::Unsupported>,
      CF::PropertyValue<PS::KeyValueCapacityReportingSupport,PS::Support::Unsupported>,
      CF::PropertyValue<PS::KeyValueInvocationConcurrency,PS::InvocationConcurrency::CallerSerialized>,
      CF::PropertyValue<PS::KeyValueFailurePreservation,PS::FailurePreservation::MayModify>,
      CF::PropertyValue<PS::KeyValueInterruptionAtomicity,PS::InterruptionAtomicity::PowerLoss>
    >>, CF::Contract<>> {
    std::array<std::uint8_t,185U> bytes{};
    bool stored=false,dropWrite=false;
    int writes=0;
    bool IsKeyValueStorageReady() const noexcept{return true;}
    PS::KeyValueSizeResult GetValueSize(PS::KeyView) const noexcept {
        if(!stored)return {PS::KeyValueSizeStatus::NotFound,PS::StorageSize{}};
        return {PS::KeyValueSizeStatus::Succeeded,PS::StorageSize{bytes.size()}};
    }
    PS::KeyValueReadResult ReadValue(PS::KeyView,PS::DestinationBufferView dst) const noexcept {
        if(!stored)return {PS::KeyValueReadStatus::NotFound,0,0,PS::StorageSize{}};
        if(dst.Capacity<bytes.size())return {PS::KeyValueReadStatus::IoFailure,0,0,PS::StorageSize{}};
        std::memcpy(dst.Address,bytes.data(),bytes.size());
        return {PS::KeyValueReadStatus::Succeeded,0,bytes.size(),PS::StorageSize{bytes.size()}};
    }
    PS::KeyValueStoreStatus StoreValue(PS::KeyView,PS::SourceBufferView src) noexcept {
        ++writes;
        if(src.Size!=bytes.size())return PS::KeyValueStoreStatus::ValueTooLarge;
        if(!dropWrite){
            std::memcpy(bytes.data(),src.Address,bytes.size());
            stored=true;
        }
        return PS::KeyValueStoreStatus::Succeeded;
    }
    PS::KeyValueRemoveStatus RemoveKey(PS::KeyView) noexcept {
        return PS::KeyValueRemoveStatus::NotFound;
    }
};
static_assert(PS::KeyValueStorageProvider<MemoryStore>);
struct HostRandom final{
 EP::Randomness::RandomByteGenerationResult FillRandomBytes(std::uint8_t* p,std::size_t n) noexcept {
    return getrandom(p,n,0)==static_cast<ssize_t>(n) ?
        EP::Randomness::RandomByteGenerationResult::Succeeded :
        EP::Randomness::RandomByteGenerationResult::ProviderFailure;
 }
};
static ID Device(unsigned last) {
    ID::Storage b{};b.back()=static_cast<std::uint8_t>(last);
    return ID{b};
}
using Source=EP::ESPIDF::Security::DevelopmentP256KeySource<MemoryStore,HostRandom>;
using Signer=EP::ESPIDF::Security::EcdsaP256Sha256Provider<Source,HostRandom>;
int main() {
    HostRandom random;
    const ES::DigitalSigningKeyReference key{17U,1U};
    MemoryStore persistent;
    std::array<std::uint8_t,65U> pub{};
    {
        Source source{persistent,random,Device(11U),42U,77U,key};
        assert(source.Recover()==Result::Unprovisioned);
        assert(!source.IsReady());
        assert(source.ResolveSigningScalar(key)==nullptr);
        assert(source.ProvisionOnce()==Result::Ready);
        assert(source.IsReady());
        assert(persistent.stored && persistent.writes==1);
        assert(source.CopyPublicKey(pub));
        assert(pub[0]==0x04U);
        assert(source.ProvisionOnce()==Result::Refused);
        Signer signer(source,random);
        const std::uint8_t message[]="EDP.H362.LABONLY.BOUND.VOTER";
        std::array<std::uint8_t,64U> sig{};
        assert(signer.Sign(key,{message,sizeof(message)},{sig.data(),sig.size()})==
            ES::DigitalSignatureSignResult::Succeeded);
        assert(signer.Verify({pub.data(),pub.size()},{message,sizeof(message)},
            {sig.data(),sig.size()})==ES::DigitalSignatureVerifyResult::Verified);
    }
    // Simulated power cycle: reconstruct from persisted bytes.
    {
        Source rebooted{persistent,random,Device(11U),42U,77U,key};
        assert(rebooted.Recover()==Result::Ready);
        std::array<std::uint8_t,65U> pub2{};
        assert(rebooted.CopyPublicKey(pub2) && pub==pub2);
        assert(rebooted.ResolveSigningScalar({17U,2U})==nullptr);
        assert(persistent.writes==1);
    }
    {Source wrong{persistent,random,Device(12U),42U,77U,key};
     assert(wrong.Recover()==Result::WrongOwner && !wrong.IsReady());}
    {Source wrong{persistent,random,Device(11U),43U,77U,key};
     assert(wrong.Recover()==Result::WrongConfiguration && !wrong.IsReady());}
    {Source wrong{persistent,random,Device(11U),42U,77U,{17U,2U}};
     assert(wrong.Recover()==Result::WrongGeneration && !wrong.IsReady());}
    {
        MemoryStore fakeSuccess;
        fakeSuccess.dropWrite=true;
        Source reject{fakeSuccess,random,Device(11U),42U,77U,key};
        assert(reject.ProvisionOnce()==Result::Faulted);
        assert(!reject.IsReady());
    }
    {
        MemoryStore corrupt=persistent;
        corrupt.bytes[70U]^=1U;
        Source reject{corrupt,random,Device(11U),42U,77U,key};
        assert(reject.Recover()==Result::Faulted && !reject.IsReady());
    }
    // Two actual independently sampled P-256 scalars for distinct owners.
    // A signature by B MUST NOT verify against C's enrolled public key.
    {
        MemoryStore storeB,storeC;
        Source b{storeB,random,Device(22U),42U,77U,key};
        Source c{storeC,random,Device(33U),42U,77U,key};
        assert(b.ProvisionOnce()==Result::Ready);
        assert(c.ProvisionOnce()==Result::Ready);
        std::array<std::uint8_t,65U> pubB{},pubC{};
        assert(b.CopyPublicKey(pubB) && c.CopyPublicKey(pubC));
        assert(pubB!=pubC);
        Signer signerB(b,random);
        const std::uint8_t vote[]="EDP.H363.PINNED.B.VOTE.v1";
        std::array<std::uint8_t,64U> signature{};
        assert(signerB.Sign(key,{vote,sizeof(vote)},{signature.data(),signature.size()})==
            ES::DigitalSignatureSignResult::Succeeded);
        assert(signerB.Verify({pubB.data(),pubB.size()},{vote,sizeof(vote)},
            {signature.data(),signature.size()})==ES::DigitalSignatureVerifyResult::Verified);
        assert(signerB.Verify({pubC.data(),pubC.size()},{vote,sizeof(vote)},
            {signature.data(),signature.size()})==ES::DigitalSignatureVerifyResult::InvalidSignature);
    }
    std::puts("H362_REAL_P256_LAB_GENERATE_COMMIT_READBACK_REBOOT_WRONG_OWNER_WRONG_GENERATION_PASS_NO_ELECTION");
}
