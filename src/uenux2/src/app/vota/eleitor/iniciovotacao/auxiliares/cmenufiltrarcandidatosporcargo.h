// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.h (path
// inferred from the .cpp, attested by srcloc line 44).
//
// RTTI: comum::CAppState <- vota::CMenuFiltrarCandidatosPorCargo (typeinfo @1545508, vtable @1545456)
//   [0] 174 [1] 144 [2] StartState 11888 [3..8] defaults
#pragma once

#include "comum/cappstate.h"

namespace vota {

class CMenuFiltrarCandidatosPorCargo final : public comum::CAppState {
public:
    /// Created inline by CMenuVisualizarCandidatos::StartState: @1834576 (mutex @1834552), CAppState(2).
    static CMenuFiltrarCandidatosPorCargo& GetInst();
    void StartState() override;       // wasm func 11888 (srcloc 44)
};

}  // namespace vota
