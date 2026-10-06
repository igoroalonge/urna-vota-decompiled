// Reconstructed from vota_web_wasm.wasm (unit u39; constructor by u10, ProcessInput by u27).
// Original (path inferred by u10/u27): uenux2/src/app/vota/operador/confirmaidentidade/cpedeanonascimentosembiometria.h
//
// "Pede ano de nascimento sem biometria": release of a voter WITHOUT usable biometrics (no fingerprints in
// the roll, or biometrics disabled): the mesário types the voter's birth year, which must match the roll.
// One retry ("ANO DE NASCIMENTO INCORRETO / CONFIRMA: tentar novamente"); after the second mismatch the
// habilitação is cancelled. Match -> CInformaEleitorPodeVotar (release the voter terminal).
//
// RTTI: comum::CAppState <- vota::CPedeAnoNascimentoSemBiometria (typeinfo @1591540, vtable @1591504)
//   [0] ICF 448  [1] ICF 765  [2] StartState 10490  [7] ProcessInput 10489 (u27)
// Lazy singleton @1908924; GetInst + ctor inlined into CNomeEleitor (func 10500).
//
// WEB BUILD: dead code in the page; reached by the operator harness (tools/bu/operator_harness.mjs, u10 §2).
#pragma once

#include <memory>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

class CPedeAnoNascimentoSemBiometria final : public comum::CAppState {
public:
    static CPedeAnoNascimentoSemBiometria& GetInst();

    void StartState() override;                        // [2] wasm func 10490
    void ProcessInput() override;                      // [7] wasm func 10489 (u27)

private:
    CPedeAnoNascimentoSemBiometria();                  // CAppState(2), see u10-foreign-fragments.cpp

    using TFormMT = std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>>;
    TFormMT m_formPedeAno;         // +12 "Digite o ANO de nascimento:" + 4-digit input
    TFormMT m_formAnoIncorreto;    // +20 "ANO DE NASCIMENTO INCORRETO / CONFIRMA: tentar novamente"
    int     m_erros;               // +28 mismatches so far          (u10: m_tentativas)
    bool    m_pedindoAno;          // +32 m_formPedeAno is on screen (u10: m_primeiraVez); sizeof 36
};

}  // namespace vota
