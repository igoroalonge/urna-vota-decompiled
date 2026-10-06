// Reconstructed from vota_web_wasm.wasm (unit u06).
// Original: uenux2/src/app/vota/eleitor/comum/cpoliticaexecucaoeleitor.cpp
//
// "Execution policy" of the voter thread: a polymorphic singleton so that the web build can replace
// the urna behaviour. Only one virtual: LimpaBufferInput() — flush the keypad buffer, used after a
// CORRIGE on the confirmation screen (CConfirmaVotoNominal / CMajoritarioValido slot 17, func 5925:
// "Eleitor corrigiu na tela de confirmação de candidato").
//
// RTTI: vota::impl::IPoliticaExecucaoEleitor
//         <- vota::impl::CPoliticaExecucaoEleitor            (urna; typeinfo @1536388, vtable @1536376)
//         <- (anonymous namespace)::CPoliticaExecucaoEleitorWeb  (vota_web_wasm.cpp:361, func 10835)
// IPoliticaExecucaoEleitor::GetInst() (srcloc :45) is inlined into func 5925: it pushes a
// CPoliticaExecucaoEleitor only if no implementation was registered. main() of the web build
// registers CPoliticaExecucaoEleitorWeb first, so the function below never runs in the simulator.
//
// srcloc: :33 / :34 IRng lookups, :37 IInputKbd lookup (virtual void ... LimpaBufferInput() const)

#include "api/hwil/iinputkbd.h"
#include "api/pattern/cpolysingleton.h"
#include "ecourna/api/security/irng.hpp"

namespace vota::impl {

class IPoliticaExecucaoEleitor {
public:
    virtual ~IPoliticaExecucaoEleitor() = default;
    virtual void LimpaBufferInput() const = 0;              // slot 2
    static IPoliticaExecucaoEleitor& GetInst();             // srcloc :45
};

class CPoliticaExecucaoEleitor final : public IPoliticaExecucaoEleitor {
public:
    void LimpaBufferInput() const override;
};

// wasm func 13564 — vtable slot 2
void CPoliticaExecucaoEleitor::LimpaBufferInput() const
{
    using ecourna::api::security::IRng;
    // IRng slot 3 returns an int; the remainders are signed (i32.rem_s), so a negative random
    // value gives 0..2 passes and a delay that wraps through the uebyte cast.
    const int repeticoes = IRng::GetInst().Gera() % 4 + 3;                     // :33  3..6 passes
    const uebyte esperaMs = static_cast<uebyte>(IRng::GetInst().Gera() % 100 + 50);   // :34  50..149 ms

    for (int i = 0; i != repeticoes; ++i) {
        api::IInputKbd::GetInst().Clear();                                     // :37  IInputKbd slot 4
        // std::this_thread::sleep_for in the urna build; here emscripten_sleep behind the global
        // flag @1584624 (=1). Without Asyncify this call aborts; see the doc (dead in the web build).
        api::Sleep(std::chrono::milliseconds(esperaMs));
    }
}

// For comparison, the web replacement (func 10835, vota_web_wasm.cpp:361, not in this unit):
//   void CPoliticaExecucaoEleitorWeb::LimpaBufferInput() const { api::IInputKbd::GetInst().Clear(); }

}  // namespace vota::impl

// ---------------------------------------------------------------------------------------------------
// Added by unit u41. The registry query of IPoliticaExecucaoEleitor::GetInst() (srcloc :45; inlined into
// func 5925, which tests it twice: once before the lazy push and once in CPolySingleton<>::instance,
// cpolysingleton.h:78):
// wasm func 2744 = api::CPolySingletonList::exists<vota::impl::IPoliticaExecucaoEleitor>(info)  (name inferred)
//   return <merged exists body, func 3884>(info, &typeid(IPoliticaExecucaoEleitor).__type_name /* @1536364 */);
// Observed executing (profiler edge 9507 -> 2744: main registering CPoliticaExecucaoEleitorWeb). Also called,
// result discarded, by __wasm_call_ctors.
template bool api::CPolySingletonList::exists<vota::impl::IPoliticaExecucaoEleitor>(api::TPolySingletonsInfo&);
