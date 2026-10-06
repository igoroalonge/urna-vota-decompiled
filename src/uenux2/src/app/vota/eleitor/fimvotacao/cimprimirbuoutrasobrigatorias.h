// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.h (path inferred from the
// .cpp, attested by srcloc lines 50, 53, 57).
//
// "Imprimir BU outras obrigatórias" = after the result files were copied to the MR, print the remaining
// MANDATORY copies (vias obrigatórias) of the BU; their number comes from the election configuration
// (CInformacaoEleicao, 1 in demonstration mode). Each copy is announced on a "telaDestinoBUs" screen
// that also lists where each copy must go (configuration text at CConfiguracaoEleicao +260).
// Then: Boletim de Justificativa (if configured) -> Boletim de Identificação de Mesários (if
// configured) -> "retire a mídia de resultado".
//
// RTTI: comum::CAppState <- vota::CImprimirBUOutrasObrigatorias (typeinfo @1541832, vtable @1541748)
//   [0] 174 [1] 144 [2] StartState 12065 [3..8] CAppState defaults
#pragma once

#include "comum/cappstate.h"

namespace vota {

class CImprimirBUOutrasObrigatorias final : public comum::CAppState {
public:
    /// Lazy singleton, wasm func 5984 (unit u37). 12 bytes, CAppState(0).
    static CImprimirBUOutrasObrigatorias& GetInst();

    void StartState() override;       // wasm func 12065 (srcloc 50, 53, 57)
};

}  // namespace vota
