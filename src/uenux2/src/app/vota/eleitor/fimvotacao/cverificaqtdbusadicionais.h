// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cverificaqtdbusadicionais.h (path inferred from the .cpp,
// attested by srcloc lines 36, 37).
//
// "Verifica quantidade de BUs adicionais" = routing state at "fim dos trabalhos": if the maximum number
// of BU copies (mandatory + additional, from the configuration) was reached, show CLimiteCopiasBUAtingido,
// else ask how many extra copies to print (CEmitirMaisBU, unit u08).
//
// RTTI: comum::CAppState <- vota::CVerificaQtdBUsAdicionais (typeinfo @1542992, vtable @1542924)
//   [0] 174 [1] 144 [2] StartState 12023 [3..8] defaults
#pragma once

#include "comum/cappstate.h"

namespace vota {

class CVerificaQtdBUsAdicionais final : public comum::CAppState {
public:
    static CVerificaQtdBUsAdicionais& GetInst();   // lazy singleton, not in this unit; 12 bytes, CAppState(0)

    void StartState() override;                    // wasm func 12023 (srcloc 36, 37)
};

}  // namespace vota
