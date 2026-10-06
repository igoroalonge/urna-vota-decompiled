// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.h (path inferred
// from the .cpp, attested by srcloc line 55).
//
// RTTI: comum::CAppState <- vota::CMenuVisualizarCandidatos (typeinfo @1545652, vtable @1545600)
//   [0] 174 [1] 144 [2] StartState 11881 [3..8] defaults
#pragma once

#include "comum/cappstate.h"

namespace vota {

class CMenuVisualizarCandidatos final : public comum::CAppState {
public:
    /// Lazy singleton, api_f2288: vota_f764(@1834608, @1834632, vtable @1545600, flags 2).
    static CMenuVisualizarCandidatos& GetInst();
    void StartState() override;       // wasm func 11881 (srcloc 55)
};

}  // namespace vota
