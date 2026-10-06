// Reconstructed from vota_web_wasm.wasm (unit u08).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cemitirmaisbu.h (path inferred from cemitirmaisbu.cpp)
//
// "Emitir mais BU" = at the very end of the election day (after the mandatory copies of the Boletim
// de Urna were printed and the result media was removed) the operator (mesário) is asked how many
// additional copies ("vias adicionais") of the BU he wants. Typing a number and CONFIRMA prints them;
// CORRIGE (or an empty field) skips. Next state: CMostraQRCodeBU.
//
// RTTI: api::CState <- comum::CAppState <- vota::CEmitirMaisBU (typeinfo @1541264, vtable @1541180)
//   [0] 174 (trivial dtor)  [1] 144 (operator delete)  [2] StartState 12082  [3] NeedChangeState 7480
//   [4] GetNextState 1661   [5] FinishState nop 218     [6] ProcessMessage nop 425
//   [7] ProcessInput 12081  [8] ProcessTick nop 425
#pragma once

#include "comum/cappstate.h"

namespace vota {

class CEmitirMaisBU final : public comum::CAppState {
public:
    /// Lazy singleton, wasm func 3879 (not in this unit): vota_f764(mutex @1833600, &s_inst @1833624,
    /// vtable, flags 2 = keyboard). 12 bytes = CAppState only.
    static CEmitirMaisBU& GetInst();

    void StartState() override;       // wasm func 12082 (srcloc 46, 47)
    void ProcessInput() override;     // wasm func 12081 (srcloc 64)

private:
    CEmitirMaisBU() : comum::CAppState(2 /* keyboard */) {}
};

}  // namespace vota
