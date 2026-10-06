// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original (path inferred): uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.h
//
// "Digital reconhecida": the voter's fingerprint matched. The MT shows "ELEITOR(A) RECONHECIDO(A)" /
// "Não assinar o caderno de votação" (a biometrically identified voter does not sign the paper roll),
// optionally the matcher score, and CONFIRMA releases the urna.
//
// RTTI: comum::CAppState <- vota::CDigitalReconhecida (typeinfo @1591700, vtable @1591648)
//   slot 0 icf 448  1 icf 765  2 StartState (10482)  7 ProcessInput (10481)  others: CAppState defaults
#pragma once

#include <memory>
#include <string>

#include "comum/cappstate.h"
#include "api/gui/cinteractiveform.h"

namespace vota {

class CDigitalReconhecida final : public comum::CAppState {
public:
    /// Lazy singleton (@1908980, 28 bytes); GetInst + constructor are inlined into
    /// CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico (func 5397).
    static CDigitalReconhecida& GetInst();

    void StartState() override;                              // slot 2 (func 10482) (srcloc line 47)
    void ProcessInput() override;                            // slot 7 (func 10481)

private:
    CDigitalReconhecida();

    bool                                  m_exibeScore;      // +11  ParametrosUrna.exibirScoreBiometria (cfg +487)
    std::shared_ptr<std::string>          m_textoScore;      // +12/+16  "Score: N" (initially " ")
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +20/+24
};

}  // namespace vota
