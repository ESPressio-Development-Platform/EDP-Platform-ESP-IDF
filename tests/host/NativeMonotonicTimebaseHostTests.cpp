#include <cassert>
#include <cstdint>
#include <cstdio>
#include <type_traits>
#include <initializer_list>
#include <timing/EspTimerMonotonicTimebaseProvider.hpp>
#include <clock/MonotonicClockProvider.hpp>

namespace T = ESPressio::Platform::ESPIDF::Timing;
using Native = T::EspTimerMonotonicTimebaseProvider;
using Result = T::TimebaseSpecificationInspectionResult;
namespace Model {
    std::int64_t Count=123'456;
    std::uint32_t Crystal=40'000'000U,Apb=80'000'000U;
    unsigned CounterReads=0U,ClockQueries=0U,DebugQueries=0U;
    bool Debug=false,FailCrystal=false,FailApb=false;
    void Reset() noexcept {
        Count=123'456;Crystal=40'000'000U;Apb=80'000'000U;
        CounterReads=ClockQueries=DebugQueries=0U;
        Debug=FailCrystal=FailApb=false;
    }
}
extern "C" std::int64_t esp_timer_get_time(void) { ++Model::CounterReads;return Model::Count; }
extern "C" bool esp_cpu_dbgr_is_attached(void) { ++Model::DebugQueries;return Model::Debug; }
extern "C" esp_err_t esp_clk_tree_src_get_freq_hz(soc_module_clk_t source,
    esp_clk_tree_src_freq_precision_t precision,std::uint32_t* output) {
    assert(precision==ESP_CLK_TREE_SRC_FREQ_PRECISION_CACHED && output!=nullptr);
    ++Model::ClockQueries;
    if (source==SOC_MOD_CLK_XTAL) {
        if(Model::FailCrystal)return ESP_FAIL;
        *output=Model::Crystal;
    } else {
        assert(source==SOC_MOD_CLK_APB);
        if(Model::FailApb)return ESP_FAIL;
        *output=Model::Apb;
    }
    return ESP_OK;
}
int main() {
    static_assert(std::is_empty_v<Native> && sizeof(Native)==1U);
    static_assert(Native::FrequencyNumerator==1'000'000U && Native::FrequencyDenominator==1U);
    static_assert(Native::SuggestedAbsoluteRateErrorPpm==150U && Native::NominalResolutionNanoseconds==1'000U);
    Native source;
    assert(Model::CounterReads==0U && Model::ClockQueries==0U && Model::DebugQueries==0U);
    const auto result=source.InspectSpecificationPrerequisites();
    assert(result==static_cast<Result>(EDP_MODEL_EXPECTED_BUILD_RESULT));
#if EDP_MODEL_EXPECTED_BUILD_RESULT != 0
    assert(Model::CounterReads==0U && Model::ClockQueries==0U && Model::DebugQueries==0U);
    std::puts("H467_BUILD_VARIANT_PREREQUISITES_DENIED|sdk=MODEL|physical_rate_measured=0");
#else
    assert(Model::CounterReads==1U && Model::ClockQueries==(Native::CounterSource()==T::EspTimerCounterSource::Esp32ApbLac?2U:1U));
    Model::Reset();Model::Debug=true;
    assert(source.InspectSpecificationPrerequisites()==Result::DebuggerAttached);
    assert(Model::ClockQueries==0U && Model::CounterReads==0U);
    Model::Reset();Model::FailCrystal=true;
    assert(source.InspectSpecificationPrerequisites()==Result::ClockFrequencyQueryFailed);
    assert(Model::CounterReads==0U);
    Model::Reset();Model::Crystal=26'000'000U;
    assert(source.InspectSpecificationPrerequisites()==Result::CrystalFrequencyMismatch);
    assert(Model::CounterReads==0U);
    Model::Reset();Model::FailApb=true;
    if constexpr(Native::CounterSource()==T::EspTimerCounterSource::Esp32ApbLac) {
        assert(source.InspectSpecificationPrerequisites()==Result::ClockFrequencyQueryFailed);
        assert(Model::CounterReads==0U);
    } else {
        assert(source.InspectSpecificationPrerequisites()==Result::PrerequisitesMatched);
        assert(Model::ClockQueries==1U); // S3 timebase is XTAL-backed, not APB-backed.
    }
    Model::Reset();Model::Apb=40'000'000U;
    assert(source.InspectSpecificationPrerequisites()==
        (Native::CounterSource()==T::EspTimerCounterSource::Esp32ApbLac?Result::ApbFrequencyMismatch:Result::PrerequisitesMatched));
    Model::Reset();Model::Count=0;
    assert(source.InspectSpecificationPrerequisites()==Result::CounterUnavailable);
    Model::Reset();Model::Count=-1;
    assert(source.InspectSpecificationPrerequisites()==Result::CounterUnavailable);
    assert(source.CurrentCount()==0U);
    Model::Reset();
    // Exact old application wrapper semantics at the signed SDK counter boundary.
    for (const auto count : {INT64_MIN, std::int64_t{-1}, std::int64_t{0},
                             std::int64_t{1}, std::int64_t{123456}, INT64_MAX}) {
        Model::Count=count;
        const auto oldWrapper=count>0?static_cast<std::uint64_t>(count):0U;
        const auto reads=Model::CounterReads;
        assert(source.CurrentCount()==oldWrapper && Model::CounterReads==reads+1U);
    }
    Model::Reset();
    ESPressio::Clock::MonotonicClockProvider<Native> canonical(source);
    assert(canonical.Now().Nanoseconds()==123'456'000U);
    Model::Count+=100;
    assert(canonical.Now().Nanoseconds()==123'556'000U);
    assert(source.CurrentCount()==123'556U && Model::ClockQueries==0U);
    std::puts("H467_SAME_CANONICAL_COUNTER_TYPED_SOURCE_CONFIG_DEBUG_AND_ZERO_REFUSALS_PASS|sdk=MODEL|hardware=0|physical_rate_measured=0|authority=0");
#endif
}
