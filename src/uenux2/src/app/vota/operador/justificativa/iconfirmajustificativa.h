// Reconstructed from vota_web_wasm.wasm (unit u27).
// Original (path inferred from iconfirmajustificativa.cpp, attested by srcloc :34):
// uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.h
//
// "Confirma justificativa": asks the mesário to confirm that the identified voter will JUSTIFY his
// absence instead of voting. Line 2 explains why he cannot vote here (a label of at most 40 chars).
//
//     Título: XXXX XXXX XXXX
//     não pertence à seção 0001              <- m_frase
//
//     CORRIGE: retornar  CONFIRMA: justificar
//
// RTTI: comum::CAppState <- vota::IConfirmaJustificativa (vtable @1589956: slot 0 dtor 1688, 1 deleting
//   dtor 2748, 2 StartState 10589, 7 ProcessInput 10588 (other unit)). Subclasses (20 bytes):
//   CConfirmaJustificativa (@1589260, frase TraduzLabel("não pertence <S|a|ao|à> <SCSN>")),
//   CConfirmaJustificativaTemporario (@1589372, "está impedido de votar nesta seção"),
//   CConfirmaJustificativaTransito (@1589484, "optou por votar EM TRÂNSITO").
#pragma once

#include <memory>
#include <string>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

class IConfirmaJustificativa : public comum::CAppState {
public:
    /// wasm func 3625 (srcloc iconfirmajustificativa.cpp:34)
    explicit IConfirmaJustificativa(const std::string& frase);
    ~IConfirmaJustificativa() override;               // slot 0 (1688 -> merged body 2902), slot 1 (2748)

    void StartState() override;                       // slot 2 (func 10589)
    void ProcessInput() override;                     // slot 7 (func 10588, other unit)

protected:
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +12/+16
};

class CConfirmaJustificativa final : public IConfirmaJustificativa {              // @1905700 (mutex @1905676)
public:  static CConfirmaJustificativa& GetInst();
private: CConfirmaJustificativa();
};
class CConfirmaJustificativaTemporario final : public IConfirmaJustificativa {    // @1905756 (mutex @1905732)
public:  static CConfirmaJustificativaTemporario& GetInst();
private: CConfirmaJustificativaTemporario();
};
class CConfirmaJustificativaTransito final : public IConfirmaJustificativa {      // @1905812 (mutex @1905788)
public:  static CConfirmaJustificativaTransito& GetInst();
private: CConfirmaJustificativaTransito();
};

}  // namespace vota
