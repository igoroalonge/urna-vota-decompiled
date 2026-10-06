// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/climitecopiasbuatingido.h (path inferred from the .cpp,
// attested by srcloc line 37).
//
// "Limite de cópias do BU atingido": shown at the end of the day when the maximum number of BU copies
// (mandatory + additional) was already printed; CONFIRMA goes to the on-screen QR code of the BU.
//
// RTTI: comum::CAppState <- vota::CLimiteCopiasBUAtingido (typeinfo @1542888, vtable @1542836)
//   [0] 244 dtor (ICF) [1] 387 deleting (ICF) [2] StartState 12027 [7] ProcessInput 12026
#pragma once

#include "comum/cappstate.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

class CLimiteCopiasBUAtingido final : public comum::CAppState {
public:
    /// Singleton created inline by CVerificaQtdBUsAdicionais::StartState (@1833960, mutex @1833936):
    /// CAppState(2), m_tela = CTelasVota::GetInst() +108/+112.
    static CLimiteCopiasBUAtingido& GetInst();

    void StartState() override;       // wasm func 12027
    void ProcessInput() override;     // wasm func 12026

private:
    CFormInterativoTelaVota m_tela;   // +12 (+16)
};

}  // namespace vota
