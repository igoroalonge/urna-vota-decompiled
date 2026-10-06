// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cquerimprimirbu.h (path inferred from the .cpp,
// attested by srcloc lines 40, 41).
//
// "Quer imprimir BU?": voter-training urnas only (CInicioBU goes here when EhTreinamentoEleitor()).
//
// RTTI: comum::CAppState <- vota::CQuerImprimirBU (typeinfo @1542648, vtable @1542580)
//   [0] 244 dtor (ICF)  [1] 387 deleting (ICF)  [2] StartState 12035  [7] ProcessInput 12034
#pragma once

#include "comum/cappstate.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

class CQuerImprimirBU final : public comum::CAppState {
public:
    /// Singleton created inline by CInicioBU::StartState (func 12062): @1833904 (mutex @1833880),
    /// CAppState(2), m_tela = CTelasVota::GetInst() +236/+240.
    static CQuerImprimirBU& GetInst();

    void StartState() override;       // wasm func 12035
    void ProcessInput() override;     // wasm func 12034

private:
    CFormInterativoTelaVota m_tela;   // +12 (+16)
};

}  // namespace vota
