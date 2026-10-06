// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporpartido.h (path
// inferred from the .cpp, attested by srcloc line 39).
//
// RTTI: comum::CAppState <- vota::CMenuFiltrarCandidatosPorPartido (typeinfo @1545580, vtable @1545528)
//   [0] 174 [1] 144 [2] StartState 11884 [3..8] defaults
#pragma once

#include "comum/cappstate.h"

namespace vota {

class CMenuFiltrarCandidatosPorPartido final : public comum::CAppState {
public:
    /// Created inline by CMenuVisualizarCandidatos::StartState: @1834604 (mutex @1834580), CAppState(2).
    static CMenuFiltrarCandidatosPorPartido& GetInst();
    void StartState() override;       // wasm func 11884 (srcloc 39)
};

}  // namespace vota
