#pragma once
// H362 DEVELOPMENT-LAB ONLY; never hardware hardened or tamper/rollback safe.
// Lab keys live in Bootstrap-provided EDP-Persistence storage (plain NVS on ESP32).
// No automatic provisioning, no private key output, no Clock authority.
#include <array>
#include <cstddef>
#include <cstdint>
#include <mbedtls/ecp.h>
#include <mbedtls/sha256.h>
#include <ESPressio_Security.hpp>
#include <ESPressio_Platform.hpp>
#include <persistence/PersistenceProviderValidation.hpp>
#include <identity/DeviceIdentifier.hpp>

namespace ESPressio::Platform::ESPIDF::Security {
enum class LabP256KeyResult : std::uint8_t {
    Ready, Unprovisioned, InvalidBootstrap, NotReady, InvalidRecord,
    WrongOwner, WrongConfiguration, WrongGeneration, CryptoFailure,
    StorageFailure, Refused, Faulted
};

template<class TStorage, class TRandom>
class DevelopmentP256KeySource final {
    static_assert(ESPressio::Persistence::KeyValueStorageProvider<TStorage>);
    using Device=ESPressio::System::Identity::DeviceIdentifier;
    using Ref=ESPressio::Security::DigitalSigningKeyReference;
    using Bytes=std::array<std::uint8_t,185U>;
    static constexpr char KeyName[]="edplabp256v1";
    static constexpr std::size_t DigestAt=153U;
    TStorage& storage_;
    TRandom& rng_;
    Device owner_{};
    std::uint64_t epoch_{},config_{};
    Ref reference_{};
    mbedtls_mpi scalar_{};
    std::array<std::uint8_t,65U> public_{};
    bool ready_{false},faulted_{false};

    static int Random(void* ctx,unsigned char* ptr,size_t n) noexcept {
        auto& r=*static_cast<TRandom*>(ctx);
        return r.FillRandomBytes(ptr,n)==
            ESPressio::Platform::Randomness::RandomByteGenerationResult::Succeeded
            ? 0 : -1;
    }
    static void Wipe(std::uint8_t* p,std::size_t n) noexcept {
        volatile std::uint8_t* q=p;
        while(n--) *q++=0U;
    }
    void Clear() noexcept {
        ready_=false;public_.fill(0U);
        mbedtls_mpi_free(&scalar_);mbedtls_mpi_init(&scalar_);
    }
    LabP256KeyResult Fault() noexcept {
        faulted_=true;Clear();return LabP256KeyResult::Faulted;
    }
    static void Write64(Bytes& b,std::size_t at,std::uint64_t v) noexcept {
        for(std::size_t i=0;i<8U;++i)b[at+i]=static_cast<std::uint8_t>(v>>(56U-8U*i));
    }
    static std::uint64_t Read64(const Bytes& b,std::size_t at) noexcept {
        std::uint64_t v=0;
        for(std::size_t i=0;i<8U;++i)v=(v<<8U)|b[at+i];
        return v;
    }
    static bool Equal(const std::uint8_t* a,const std::uint8_t* b,std::size_t n) noexcept {
        unsigned diff=0;
        for(std::size_t i=0;i<n;++i)diff|=a[i]^b[i];
        return diff==0U;
    }
    static int DerivePublic(
        const mbedtls_mpi& secret, TRandom& rng,
        std::array<std::uint8_t,65U>& out
    ) noexcept {
        mbedtls_ecp_group grp;mbedtls_ecp_point point;
        mbedtls_ecp_group_init(&grp);mbedtls_ecp_point_init(&point);
        int rc=mbedtls_ecp_group_load(&grp,MBEDTLS_ECP_DP_SECP256R1);
        if(rc==0)rc=mbedtls_ecp_check_privkey(&grp,&secret);
        if(rc==0)rc=mbedtls_ecp_mul(&grp,&point,&secret,&grp.G,Random,&rng);
        std::size_t size=0;
        if(rc==0)rc=mbedtls_ecp_point_write_binary(&grp,&point,
            MBEDTLS_ECP_PF_UNCOMPRESSED,&size,out.data(),out.size());
        if(rc==0 && (size!=out.size() || out[0]!=0x04U))rc=-1;
        mbedtls_ecp_point_free(&point);mbedtls_ecp_group_free(&grp);
        return rc;
    }
    LabP256KeyResult Load(Bytes& b) noexcept {
        using namespace ESPressio::Persistence;
        if(!storage_.IsKeyValueStorageReady())return LabP256KeyResult::NotReady;
        const auto k=KeyView::Validate(KeyName);
        if(k.Status!=KeyValidationStatus::Succeeded)return LabP256KeyResult::InvalidBootstrap;
        const auto len=storage_.GetValueSize(k.Value);
        if(len.Status==KeyValueSizeStatus::NotFound)return LabP256KeyResult::Unprovisioned;
        if(len.Status!=KeyValueSizeStatus::Succeeded ||
           len.Size.RawValue!=b.size())return LabP256KeyResult::InvalidRecord;
        const auto v=storage_.ReadValue(k.Value,DestinationBufferView{b.data(),b.size()});
        if(v.Status!=KeyValueReadStatus::Succeeded ||
           v.BytesTransferred!=b.size() ||
           HasFact(v.Facts,ReadFact::WasTruncated))return LabP256KeyResult::StorageFailure;
        return LabP256KeyResult::Ready;
    }
public:
    DevelopmentP256KeySource(
        TStorage& store,TRandom& rng,Device owner,
        std::uint64_t epoch,std::uint64_t config,Ref key
    ) noexcept :storage_(store),rng_(rng),owner_(owner),epoch_(epoch),
                config_(config),reference_(key){
        mbedtls_mpi_init(&scalar_);
    }
    DevelopmentP256KeySource(const DevelopmentP256KeySource&)=delete;
    DevelopmentP256KeySource& operator=(const DevelopmentP256KeySource&)=delete;
    ~DevelopmentP256KeySource(){Clear();mbedtls_mpi_free(&scalar_);}
    bool IsReady() const noexcept {return ready_ && !faulted_;}
    const mbedtls_mpi* ResolveSigningScalar(Ref r) const noexcept {
        return IsReady() && r.Identifier==reference_.Identifier &&
            r.Generation==reference_.Generation ? &scalar_:nullptr;
    }
    bool CopyPublicKey(std::array<std::uint8_t,65U>& out) const noexcept {
        out.fill(0U);
        if(!IsReady())return false;
        out=public_;return true;
    }
    LabP256KeyResult Recover() noexcept {
        Clear();
        if(faulted_)return LabP256KeyResult::Faulted;
        if(!owner_.IsValid() || !epoch_ || !config_ || !reference_.IsValid())
            return LabP256KeyResult::InvalidBootstrap;
        Bytes b{};
        const auto st=Load(b);
        if(st!=LabP256KeyResult::Ready){
            Wipe(b.data(),b.size());
            return st==LabP256KeyResult::Unprovisioned?st:Fault();
        }
        constexpr std::uint8_t marker[]={'E','D','P','L','A','B',1U,0U};
        std::uint8_t expected[32U]{};
        const int hash=mbedtls_sha256(b.data(),DigestAt,expected,0);
        const bool valid=hash==0 && Equal(marker,b.data(),8U) &&
            Equal(expected,b.data()+DigestAt,32U);
        Wipe(expected,sizeof(expected));
        if(!valid){Wipe(b.data(),b.size());return Fault();}
        if(!Equal(b.data()+8U,owner_.Bytes().data(),16U)){
            Wipe(b.data(),b.size());return LabP256KeyResult::WrongOwner;
        }
        if(Read64(b,24U)!=epoch_ || Read64(b,32U)!=config_){
            Wipe(b.data(),b.size());return LabP256KeyResult::WrongConfiguration;
        }
        if(Read64(b,40U)!=reference_.Identifier ||
           Read64(b,48U)!=reference_.Generation){
            Wipe(b.data(),b.size());return LabP256KeyResult::WrongGeneration;
        }
        int rc=mbedtls_mpi_read_binary(&scalar_,b.data()+56U,32U);
        std::array<std::uint8_t,65U> calculated{};
        if(rc==0)rc=DerivePublic(scalar_,rng_,calculated);
        if(rc==0 && !Equal(calculated.data(),b.data()+88U,65U))rc=-1;
        if(rc==0)public_=calculated;
        calculated.fill(0U);Wipe(b.data(),b.size());
        if(rc!=0)return Fault();
        ready_=true;return LabP256KeyResult::Ready;
    }
    // Explicit lab-only Bootstrap operation: never replace an existing key
    // and never make a newly generated scalar sign until NVS readback passes.
    LabP256KeyResult ProvisionOnce() noexcept {
        if(faulted_ || ready_)return LabP256KeyResult::Refused;
        if(Recover()!=LabP256KeyResult::Unprovisioned)return LabP256KeyResult::Refused;
        mbedtls_ecp_group group;mbedtls_ecp_group_init(&group);
        int rc=mbedtls_ecp_group_load(&group,MBEDTLS_ECP_DP_SECP256R1);
        if(rc==0)rc=mbedtls_ecp_gen_privkey(&group,&scalar_,Random,&rng_);
        std::array<std::uint8_t,65U> pub{};
        if(rc==0)rc=DerivePublic(scalar_,rng_,pub);
        mbedtls_ecp_group_free(&group);
        if(rc!=0)return Fault();
        Bytes b{};
        constexpr std::uint8_t marker[]={'E','D','P','L','A','B',1U,0U};
        for(std::size_t i=0;i<8U;++i)b[i]=marker[i];
        for(std::size_t i=0;i<16U;++i)b[8U+i]=owner_.Bytes()[i];
        Write64(b,24U,epoch_);Write64(b,32U,config_);
        Write64(b,40U,reference_.Identifier);
        Write64(b,48U,reference_.Generation);
        rc=mbedtls_mpi_write_binary(&scalar_,b.data()+56U,32U);
        if(rc==0){
            for(std::size_t i=0;i<65U;++i)b[88U+i]=pub[i];
            rc=mbedtls_sha256(b.data(),DigestAt,b.data()+DigestAt,0);
        }
        if(rc!=0){Wipe(b.data(),b.size());return Fault();}
        auto key=ESPressio::Persistence::KeyView::Validate(KeyName);
        if(key.Status!=ESPressio::Persistence::KeyValidationStatus::Succeeded){
            Wipe(b.data(),b.size());return Fault();
        }
        const auto saved=storage_.StoreValue(key.Value,
            ESPressio::Persistence::SourceBufferView{b.data(),b.size()});
        Wipe(b.data(),b.size());Clear();
        if(saved!=ESPressio::Persistence::KeyValueStoreStatus::Succeeded)
            return Fault();
        const auto verified=Recover();
        if(verified!=LabP256KeyResult::Ready || !Equal(public_.data(),pub.data(),65U))
            return Fault();
        return LabP256KeyResult::Ready;
    }
};
} // namespace ESPressio::Platform::ESPIDF::Security
