// uenux2/src/app/vota/eleitor/votaproporcional/ccompletaproporcional.h   (path inferred from
// ccompletaproporcional.cpp, attested by std::source_location records :60 and :83)
// Reconstructed from vota_web_wasm.wasm (unit u26).
//
// vota::CCompletaProporcional ("complete the proportional number") = abstract base of the two sub-states that
// follow CPedeProporcional: CPedeNominal (valid party: type the remaining digits of the candidate) and
// CPedeNulo (unknown party: the number will be a null vote, or an inapto candidate). The digits typed here
// are appended to the two party digits already in g_votoDigitado; the subclass decides the next state
// (slot 16 GetProximoEstado).
//
// RTTI: CVotacaoStateAudio <- vota::CCompletaProporcional (typeinfo @1548204, vtable @1548104)
//       <- CPedeNulo (@1548532), CPedeNominal (@1549088). 32 bytes.
//   [9] ProcessInputAudio 11756  [10] StartStateAudio 11757  [12] ProcessTickAudio (nop 425)
//   [13] EmiteEcoComInputField 5930  [15] GetMensagemAudio = 0  [16] GetProximoEstado = 0
#pragma once

#include <string>
#include <utility>

#include "vota/eleitor/cvotacaostateaudio.h"
#include "vota/eleitor/comum/ctelascargo.h"          // ETelaVotacao
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

class CCompletaProporcional : public CVotacaoStateAudio {
public:
    void ProcessInputAudio() override;                                          // [9]  func 11756
    void StartStateAudio() override;                                            // [10] func 11757
    void ProcessTickAudio(uebyte) override {}                                   // [12] nop
    std::pair<api::EInputResult, std::string>
    EmiteEcoComInputField(const CFormInterativoTelaVota& tela) const override;  // [13] func 5930 (srcloc :60)
    virtual comum::CAppState* GetProximoEstado(const std::string& numero) const = 0;   // [16] (attested by CPedeNulo)

protected:
    /// wasm func 5932 (name inferred): CVotacaoStateAudio(6) (keyboard + ticks), m_tela = tela.
    explicit CCompletaProporcional(ETelaVotacao tela) : CVotacaoStateAudio(6), m_tela(tela) {}

    /// ccompletaproporcional.cpp:83 - thunk func 5931 over the merged body func 3921 (error 9384): the screen
    /// m_tela of the current cargo; `funcao` (the caller's __PRETTY_FUNCTION__) only names the caller in the
    /// error message.
    CFormInterativoTelaVota GetTelaCargoAtual(const std::string& funcao);

    ETelaVotacao m_tela;                                  // +28 (uebyte): 8 CPedeNominal, 5 CPedeNulo
};

} // namespace vota
