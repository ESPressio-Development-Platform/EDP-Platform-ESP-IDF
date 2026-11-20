#include "ModelRecord.hpp"

#include <barrier>
#include <cstdlib>
#include <thread>

namespace ESPressio::Platform::ESPIDF::Execution::Tests {

    static_assert(
        sizeof(ESPressio::Platform::Execution::Detail::CurrentContextBindingProviderTraits<Provider>) > 0U
    );
    static_assert(std::is_trivially_destructible_v<Provider>);

    /// Keeps runtime oracles active in Release/NDEBUG.
    void Require(bool condition) {
        if (!condition) {
            std::abort();
        }
    }

    /// Exercises all statuses, preservation, interrupt and pre-scheduler rejection.
    void CheckOperations() {
        using namespace ESPressio::Platform::Execution;
        Provider provider;
        const Record first{1U};
        const Record second{2U};

        Require(provider.ReadCurrent().Status() == CurrentContextReadStatus::Unbound);
        Require(provider.ReadCurrent().GetIfBound() == nullptr);
        Require(provider.RetireCurrent(first) == CurrentContextRetireResult::NotBound);
        ModelSchedulerState = taskSCHEDULER_NOT_STARTED;
        Require(provider.BindCurrent(first) == CurrentContextBindResult::UnsupportedContext);
        Require(provider.ReadCurrent().Status() == CurrentContextReadStatus::UnsupportedContext);
        Require(provider.ReadCurrent().GetIfBound() == nullptr);
        Require(provider.RetireCurrent(first) == CurrentContextRetireResult::UnsupportedContext);
        ModelSchedulerState = taskSCHEDULER_RUNNING;
        Require(provider.BindCurrent(first) == CurrentContextBindResult::Bound);
        Require(provider.BindCurrent(second) == CurrentContextBindResult::AlreadyBound);
        Require(provider.ReadCurrent().IsBound());
        Require(provider.ReadCurrent().GetIfBound() == &first);
        Require(ReadFromOtherTranslationUnit() == &first);
        Require(provider.RetireCurrent(second) == CurrentContextRetireResult::DifferentBinding);
        Require(provider.ReadCurrent().GetIfBound() == &first);
        ModelInIsr = true;
        Require(provider.ReadCurrent().Status() == CurrentContextReadStatus::UnsupportedContext);
        Require(provider.ReadCurrent().GetIfBound() == nullptr);
        Require(provider.BindCurrent(second) == CurrentContextBindResult::UnsupportedContext);
        Require(provider.RetireCurrent(first) == CurrentContextRetireResult::UnsupportedContext);
        ModelInIsr = false;
        Require(provider.ReadCurrent().GetIfBound() == &first);
        ModelSchedulerState = taskSCHEDULER_SUSPENDED;
        Require(provider.ReadCurrent().GetIfBound() == &first);
        Require(provider.RetireCurrent(first) == CurrentContextRetireResult::Retired);
        Require(provider.ReadCurrent().GetIfBound() == nullptr);
        ModelSchedulerState = taskSCHEDULER_RUNNING;
    }


    /// Models owner lifetime only; actual Threading entry integration is separately qualified.
    class OwnerScope final {

        private:

            /// Explicitly selected provider.
            Provider& _provider;

            /// Caller record outlives this scope.
            const Record& _record;

        public:

            /// Binds before simulated captured work.
            OwnerScope(
                Provider& provider,
                const Record& record
            ) noexcept
                : _provider(provider),
                  _record(record) {
                Require(_provider.BindCurrent(_record) == ESPressio::Platform::Execution::CurrentContextBindResult::Bound);
            }

            /// Retires after inner scopes unwind.
            ~OwnerScope() noexcept {
                Require(_provider.RetireCurrent(_record) == ESPressio::Platform::Execution::CurrentContextRetireResult::Retired);
            }

            /// Prevents duplicated scope ownership.
            OwnerScope(const OwnerScope&) = delete;

            /// Prevents reassigned scope ownership.
            OwnerScope& operator =(const OwnerScope&) = delete;

    };


    /// Tests ordinary and exception exits in the model, never native task cancellation.
    void CheckUnwinding() {
        Provider provider;
        const Record record{3U};
        {
            OwnerScope scope(
                provider,
                record
            );
            Require(provider.ReadCurrent().GetIfBound() == &record);
        }
        Require(!provider.ReadCurrent().IsBound());
        try {
            OwnerScope scope(
                provider,
                record
            );
            throw 17;
        } catch (int value) {
            Require(value == 17);
        }
        Require(!provider.ReadCurrent().IsBound());
    }


    /// Tests concurrent TLS isolation and shared inline identity across translation units.
    void CheckThreadIsolation() {
        using namespace ESPressio::Platform::Execution;
        std::barrier rendezvous(3);
        Provider provider;
        auto entry = [&](std::uint64_t identity) {
            const Record record{identity};
            Require(provider.ReadCurrent().Status() == CurrentContextReadStatus::Unbound);
            Require(provider.BindCurrent(record) == CurrentContextBindResult::Bound);
            rendezvous.arrive_and_wait();
            for (unsigned round = 0U; round < 10000U; ++round) {
                Require(provider.ReadCurrent().GetIfBound() == &record);
                Require(ReadFromOtherTranslationUnit() == &record);
                std::this_thread::yield();
            }
            rendezvous.arrive_and_wait();
            Require(provider.RetireCurrent(record) == CurrentContextRetireResult::Retired);
            Require(provider.ReadCurrent().Status() == CurrentContextReadStatus::Unbound);
        };
        std::thread first(
            entry,
            11U
        );
        std::thread second(
            entry,
            12U
        );
        rendezvous.arrive_and_wait();
        Require(provider.ReadCurrent().Status() == CurrentContextReadStatus::Unbound);
        rendezvous.arrive_and_wait();
        first.join();
        second.join();

        for (std::uint64_t identity = 13U; identity < 29U; ++identity) {
            std::thread fresh(
                [&, identity]() {
                    const Record record{identity};
                    Require(provider.ReadCurrent().Status() == CurrentContextReadStatus::Unbound);
                    Require(provider.BindCurrent(record) == CurrentContextBindResult::Bound);
                    Require(provider.ReadCurrent().GetIfBound()->identity == identity);
                    Require(provider.RetireCurrent(record) == CurrentContextRetireResult::Retired);
                }
            );
            fresh.join();
        }
    }

} // ESPressio::Platform::ESPIDF::Execution::Tests


/// Runs SDK MODEL tests against actual provider source, without target qualification claims.
int main() {
    using namespace ESPressio::Platform::ESPIDF::Execution::Tests;
    CheckOperations();
    CheckUnwinding();
    CheckThreadIsolation();
}
