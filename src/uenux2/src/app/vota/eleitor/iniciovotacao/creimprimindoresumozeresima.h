// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/creimprimindoresumozeresima.h (path inferred from
// the .cpp, attested by srcloc line 60).
//
// "Reimprimindo resumo da zerésima": reprints trab/rze.dat, sets EstadoVota = zerésima impressa and
// continues with CDefineRotaPosReinicio (routing after a restart).
//
// RTTI: comum::CAppState <- vota::CReimprimindoResumoZeresima (typeinfo @1546256, vtable @1546204)
//   [0] 174 [1] 144 [2] StartState 11855 [3..8] defaults
#pragma once

#include "comum/cappstate.h"

namespace vota {

class CReimprimindoResumoZeresima final : public comum::CAppState {
public:
    /// func 5942 (not in this unit): vota_f764(@1834776, @1834800, vtable @1546204, flags 0)
    static CReimprimindoResumoZeresima& GetInst();
    void StartState() override;                // wasm func 11855
private:
    static void ImprimeResumoZeresima();       // :60, inlined
};

}  // namespace vota
