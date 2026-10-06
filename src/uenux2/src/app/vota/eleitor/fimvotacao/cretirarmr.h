// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cretirarmr.h (path inferred from cretirarmr.cpp,
// attested by srcloc lines 54, 55, 56, 78, 96).
//
// "Retirar MR" = asks the mesário to remove the MR (mídia de resultado, the USB result stick) and waits,
// polling, until it is gone; then shows the "fim dos trabalhos" screen whose CONFIRMA leads to the
// extra-copies question (CEmitirMaisBU). In demonstration mode nothing is waited for.
//
// RTTI: comum::CAppState <- vota::CRetirarMR (typeinfo @1542800, vtable @1542684)
//   [0] 1284 dtor (ICF: releases the three screens)  [1] 2884 deleting (ICF)
//   [2] StartState 12031  [7] ProcessInput 12030
#pragma once

#include "comum/cappstate.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

class CRetirarMR final : public comum::CAppState {
public:
    /// Lazy singleton, wasm func 2291 (unit u?): CAppState(2) and three screens built in place
    /// ("Retire a mídia de resultado", "Mantenha a mídia de resultado", "telaMRModoDemo", "Lacre a
    /// tampa do compartimento" ...).
    static CRetirarMR& GetInst();

    void StartState() override;       // wasm func 12031 (srcloc 54, 55, 56; AguardaRetiradaMR :96 inlined)
    void ProcessInput() override;     // wasm func 12030 (srcloc 78)

private:
    bool AguardaRetiradaMR();         // srcloc :96, inlined into StartState

    CFormInterativoTelaVota m_telaRetireMR;        // +12 (+16) "Retire a mídia de resultado"
    CFormInterativoTelaVota m_telaFimTrabalhos;    // +20 (+24) shown after the MR was removed  name inferred
    CFormInterativoTelaVota m_telaModoDemo;        // +28 (+32) "telaMRModoDemo"            name inferred
};

}  // namespace vota
