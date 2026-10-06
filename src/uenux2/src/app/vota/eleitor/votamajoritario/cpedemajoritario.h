// uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.h   (path inferred from cpedemajoritario.cpp,
// attested by std::source_location records :44, :64, :125)
// Reconstructed from vota_web_wasm.wasm (unit u26 = owner; the shared body of GetTelaCargoAtual, func 6050,
// was written by unit u07 in cpedemajoritario.u07.cpp).
//
// vota::CPedeMajoritario ("ask for a majoritarian vote") is the voter sub-state that waits for the number of
// a candidate for a majoritarian office (Prefeito, Governador, Senador, Presidente) or the answer of a
// referendum ("consulta"). The screen's input field has as many digits as the office; the vote is typed
// until the field is full (EInputResult 9) or BRANCO is pressed (3). The result goes to a conferência
// state CConfereVotoEmCargo<TConfirma, ETelaVotacao> (unit u06).
//
// RTTI: comum::CAppState <- vota::CVotacaoStateAudio <- vota::CPedeMajoritario (typeinfo @1550400,
// vtable @1550288). 28 bytes (no members of its own); singleton = func 5921 (merged body 6051, flags 2).
//   [9] ProcessInputAudio 11683   [10] StartStateAudio 11684   [13] CVotacaoStateAudio::EmiteEcoComInputField
//   [15] GetMensagemAudio 11682.  (Slot names 9/10 are attested by the __PRETTY_FUNCTION__ strings passed by
//   CCompletaProporcional to GetTelaCargoAtual.)
#pragma once

#include <string>

#include "vota/eleitor/cvotacaostateaudio.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

class CPedeMajoritario : public CVotacaoStateAudio {
public:
    static CPedeMajoritario& GetInst();                          // func 5921 (unit u06)

    void ProcessInputAudio() override;                           // [9]  func 11683 (srcloc :64)
    void StartStateAudio() override;                             // [10] func 11684
    std::string GetMensagemAudio() const override;               // [15] func 11682

private:
    CPedeMajoritario() : CVotacaoStateAudio(2) {}

    /// cpedemajoritario.cpp:44 - thunk func 5920 over the merged body func 6050 (error 9380).
    CFormInterativoTelaVota GetTelaCargoAtual();

    /// cpedemajoritario.cpp:125 - inlined into 11683.
    bool ExisteCandidatoValidoResposta(const std::string& numero) const;

    /// Inlined into 11683 (name inferred): same candidate already chosen for another vaga of this cargo.
    bool VotoRepetido(const std::string& numero) const;
};

} // namespace vota
