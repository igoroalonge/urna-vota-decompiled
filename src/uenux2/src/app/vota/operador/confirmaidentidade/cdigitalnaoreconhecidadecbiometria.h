// Reconstructed from vota_web_wasm.wasm (unit u39; constructor by u10, ProcessInput by u19).
// Original (path inferred by u10): uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.h
//
// "Digital não reconhecida - decifração da biometria": the voter's fingerprint templates in the roll could
// not be decrypted (CBiometriaEleitor estado de decifração > 0, i.e. ModuloResultadoUrnaCadastro
// ErroLeituraBiometria), so no comparison is possible. MT: "Eleitor(a) não reconhecido(a) / Dados
// biométricos inválidos / CORRIGE: cancelar / CONFIRMA: prosseguir".
// Entered from CDigitalNaoReconhecida::StartState (func 10474).
//
// RTTI: comum::CAppState <- vota::CDigitalNaoReconhecidaDecBiometria (typeinfo @1591772, vtable @1591736)
//   [0] ICF 244  [1] ICF 387  [2] StartState 10478  [7] ProcessInput 10477 (u19)
// Lazy singleton @1909008; GetInst + ctor inlined into func 10474.
//
// WEB BUILD: dead code (operator thread not run).
#pragma once

#include <memory>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

class CDigitalNaoReconhecidaDecBiometria final : public comum::CAppState {
public:
    static CDigitalNaoReconhecidaDecBiometria& GetInst();

    void StartState() override;                        // [2] wasm func 10478
    void ProcessInput() override;                      // [7] wasm func 10477 (u19)

private:
    CDigitalNaoReconhecidaDecBiometria();              // CAppState(2), see u10-foreign-fragments.cpp

    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +12; sizeof 20
};

}  // namespace vota
