// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbjust.h (path inferred from cimprimirbjust.cpp,
// attested by srcloc lines 51, 52, 69).
//
// "Imprimir BJust" = prints (one copy, "via única") the Boletim de Justificativa eleitoral, the report
// trab/buj.dat generated earlier by CGeraRelatorios. Only when the configuration asks for it.
//
// RTTI: comum::CAppState <- vota::CImprimirBJust (typeinfo @1541728, vtable @1541644)
//   [0] 174 [1] 144 [2] StartState 12068 (imprimeBJust inlined) [3..8] defaults
#pragma once

#include "comum/cappstate.h"

namespace vota {

class CImprimirBJust final : public comum::CAppState {
public:
    /// Singleton created inline by CImprimirBUOutrasObrigatorias::StartState: @1833736 (mutex @1833712),
    /// 12 bytes, CAppState(0).
    static CImprimirBJust& GetInst();

    void StartState() override;       // wasm func 12068 (srcloc 51, 52)

private:
    void imprimeBJust();              // srcloc :69, inlined into StartState
};

}  // namespace vota
