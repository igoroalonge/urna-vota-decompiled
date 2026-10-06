// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/cquerreimprimirzeresima.h (path inferred from the .cpp,
// attested by srcloc line 45).
//
// "Quer reimprimir zerésima?" = after a restart before voting (CReinicioVotacao), asks whether to reprint
// the zerésima: '1' + CONFIRMA = zerésima and summary, '2' + CONFIRMA = only the summary, CORRIGE with an
// empty field = go on (registro de mesários or start of voting), BRANCO = "Mais informações" menu.
// Only allowed while nobody has voted (comparecimento == 0).
//
// RTTI: comum::CAppState <- vota::CQuerReimprimirZeresima (typeinfo @1546400, vtable @1546348)
//   [0] 244 dtor (ICF) [1] 387 (ICF) [2] StartState 11849 [7] ProcessInput 11848
#pragma once

#include "comum/cappstate.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

class CQuerReimprimirZeresima final : public comum::CAppState {
public:
    static CQuerReimprimirZeresima& GetInst();   // func 5940 (fragment by unit u07)

    void StartState() override;                  // wasm func 11849 (srcloc 45)
    void ProcessInput() override;                // wasm func 11848

private:
    CQuerReimprimirZeresima();                   // CAppState(2), m_tela = CTelasVota +244

    CFormInterativoTelaVota m_tela;              // +12 (+16) m_telaQuerReimprimirZeresima
};

}  // namespace vota
