// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobehb.h (path inferred from cimprimindobehb.cpp,
// attested by srcloc lines 45, 46, 55).
//
// "Imprimindo BEHB" = prints the "boletim de eleitores habilitados biograficamente" (trab/behb.dat:
// voters released by the mesário without fingerprint match), "via única". Only on biometric urnas
// outside demonstration mode.
//
// RTTI: comum::CAppState <- vota::CImprimindoBEHB (typeinfo @1541520, vtable @1541436)
//   [0] 174 [1] 144 [2] StartState 12074 (ImprimeRelatorio inlined) [3..8] defaults
#pragma once

#include "comum/cappstate.h"

namespace vota {

class CImprimindoBEHB final : public comum::CAppState {
public:
    /// Singleton created inline by CImprimindoBim::StartState: @1833680 (mutex @1833656), CAppState(0).
    static CImprimindoBEHB& GetInst();

    void StartState() override;       // wasm func 12074 (srcloc 45, 46)

private:
    void ImprimeRelatorio();          // srcloc :55, inlined into StartState
};

}  // namespace vota
