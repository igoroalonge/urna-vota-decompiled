// Reconstructed from vota_web_wasm.wasm (unit u39; constructor/GetInst by u33, ProcessInput by u19).
// Original (path inferred by u33): uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.h
//
// "Habilita áudio do eleitor": the voter being released needs the accessible (audio) vote - registration
// flag "necessidade especial" or audio switched on by hand in "outras opções". The MT asks the mesário to
// plug the headphones into the urna ("Este eleitor necessita de áudio / Coloque o fone de ouvido na urna /
// CONFIRMA: continuar"); CONFIRMA posts messages 8/9 (audio on) and 1 (voter released) to the voter thread.
//
// RTTI: comum::CAppState <- vota::CHabilitaAudioEleitor (typeinfo @1590896, vtable @1590860, 20 bytes)
//   [0] ICF 244  [1] ICF 387  [2] StartState 10524  [7] ProcessInput 10523 (u19)
//
// WEB BUILD: dead code (operator thread not run).
#pragma once

#include <memory>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

class CHabilitaAudioEleitor final : public comum::CAppState {
public:
    static CHabilitaAudioEleitor& GetInst();           // wasm func 2743 (u33)

    void StartState() override;                        // [2] wasm func 10524
    void ProcessInput() override;                      // [7] wasm func 10523 (u19)

private:
    CHabilitaAudioEleitor();                           // CAppState(2), chabilitaaudioeleitor.u33.cpp

    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +12; sizeof 20
};

}  // namespace vota
