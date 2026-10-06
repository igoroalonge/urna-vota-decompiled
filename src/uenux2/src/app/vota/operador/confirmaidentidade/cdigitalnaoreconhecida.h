// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original (path inferred): uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.h
//
// "Digital não reconhecida": the captured fingerprint matched none of the 4 stored fingers.
// CONFIRMA tries again (next attempt) or, after the last attempt (ParametrosUrna.numTentativasHabilitacao),
// goes to the birth-year check CVerificaDadoEleitor; CORRIGE cancels the habilitação.
// If the voter's biometric data could not even be decrypted, StartState diverts to
// CDigitalNaoReconhecidaDecBiometria ("Dados biométricos inválidos").
//
// RTTI: comum::CAppState <- vota::CDigitalNaoReconhecida (typeinfo @1591860, vtable @1591808)
//   slot 0 icf 1284  1 icf 2884  2 StartState (10474)  7 ProcessInput (10473)
#pragma once

#include <memory>
#include <string>

#include "comum/cappstate.h"
#include "api/gui/cinteractiveform.h"

namespace vota {

class CDigitalNaoReconhecida final : public comum::CAppState {
public:
    /// Lazy singleton (@1909036, 36 bytes); GetInst + constructor are inlined into
    /// CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico (func 5397).
    static CDigitalNaoReconhecida& GetInst();

    void StartState() override;                              // slot 2 (func 10474) (srcloc line 82)
    void ProcessInput() override;                            // slot 7 (func 10473)

private:
    CDigitalNaoReconhecida();

    bool                                  m_exibeScore;      // +11  cfg +487
    std::shared_ptr<std::string>          m_textoTentativa;  // +12/+16 "Tentativa x de x"
    std::shared_ptr<std::string>          m_textoScore;      // +20/+24 " "
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +28/+32
};

}  // namespace vota
