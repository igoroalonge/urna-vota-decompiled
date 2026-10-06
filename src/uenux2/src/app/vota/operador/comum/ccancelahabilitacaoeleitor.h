// Reconstructed from vota_web_wasm.wasm (unit u39; GetInst by u19).
// Original (path inferred): uenux2/src/app/vota/operador/comum/ccancelahabilitacaoeleitor.h
// (include path already used 4 times by units u10/u19/u27: every step of the identification may cancel).
//
// "Cancela habilitação do eleitor": transit state entered when the mesário gives up releasing a voter
// (CORRIGE during fingerprint / birth-year / justification steps). Clears everything recorded for this
// habilitação and returns to the identification screen. No screen, no keys.
//
// RTTI: comum::CAppState <- vota::CCancelaHabilitacaoEleitor (typeinfo @1588792, vtable @1588756, 12 bytes)
//   [0] 174 (trivial dtor)  [1] 144 (operator delete)  [2] StartState 10646  [3..8] CAppState defaults
// GetInst = func 1536 (merged lazy-singleton body vota_f764, mutex @1905508).
//
// WEB BUILD: dead code (operator thread not run).
#pragma once

#include "comum/cappstate.h"

namespace vota {

class CCancelaHabilitacaoEleitor final : public comum::CAppState {
public:
    static CCancelaHabilitacaoEleitor& GetInst();      // wasm func 1536 (u19)

    void StartState() override;                        // [2] wasm func 10646

private:
    CCancelaHabilitacaoEleitor() : comum::CAppState(0) {}   // vota_f764(@1905508, @1905532, vtable, flags 0)
};

}  // namespace vota
