// Reconstructed from vota_web_wasm.wasm (unit u39; constructor by u10, ProcessInput by u17).
// Original (path inferred by u10): uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.h
//
// "Verifica dado do eleitor": after the last failed fingerprint attempt the mesário may release the voter
// manually ("habilitação manual") by typing the voter's birth year. CONFIRMA compares it with the roll:
// equal -> CRegistraDigitalOperador (the mesário's own fingerprint authorises the release), different ->
// CDadoEleitorNaoConfere; CORRIGE -> CPedeIdentidade.
//
// RTTI: comum::CAppState <- vota::CVerificaDadoEleitor (typeinfo @1592756, vtable @1592720)
//   [0] ICF 244  [1] ICF 387  [2] StartState 10444  [7] ProcessInput 10443 (u17)
// Lazy singleton func 2735 (u10).
//
// WEB BUILD: dead code (operator thread not run).
#pragma once

#include <memory>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

class CVerificaDadoEleitor final : public comum::CAppState {
public:
    static CVerificaDadoEleitor& GetInst();            // wasm func 2735 (u10)

    void StartState() override;                        // [2] wasm func 10444
    void ProcessInput() override;                      // [7] wasm func 10443 (u17)

private:
    CVerificaDadoEleitor();                            // CAppState(2)

    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +12; sizeof 20
};

}  // namespace vota
