// Reconstructed from vota_web_wasm.wasm (unit u06). Original: uenux2/src/app/vota/eleitor/cfimvotoeleitor.cpp
//
// CFimVotoEleitor = end of one voter's session after the votes were written (reached from
// CSincronismoEleitor). Shows "FIM / VOTOU", plays the end beeps, says "fim" when audio is on and
// tells the operator terminal; then the voter thread goes back to CAguardaMensagem (waiting for the
// next habilitação). The web adapter detects the end of a vote exactly at this transition
// (state name contains "CAguardaMensagem" again -> event "vota:done").
//
// RTTI: comum::CAppState <- vota::CFimVotoEleitor (typeinfo @1534064, vtable @1534012, 12 bytes)
//       vtable: [0] func 174 (trivial dtor) [1] func 144 (operator delete) [2] StartState (7198),
//       everything else inherited from CAppState.
//
// srcloc: :42 virtual void vota::CFimVotoEleitor::StartState()   (IBeep lookup)

#include "comum/cappstate.h"

#include "api/hwil/ibeep.h"
#include "vota/comum/cthreadoperador.h"
#include "vota/eleitor/celeitorvotando.h"
#include "vota/eleitor/cvotacaostateaudio.h"
#include "vota/eleitor/comum/cinformacaoeleitor.h"   // CInformacaoEleitor (func 509, unit u02)
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/operador/caguardamensagem.h"

namespace vota {

class CFimVotoEleitor final : public comum::CAppState {
public:
    void StartState() override;
};

// wasm func 7198 — vtable slot 2. Observed executing (end of every recorded vote).
void CFimVotoEleitor::StartState()
{
    CTelasVota::GetInst().m_telaFimVotou->Exibe();          // CTelasVota +156: "telaFim", "FIM"/"VOTOU"
    api::IBeep::GetInst().Beep(4);                          // :42  IBeep slot 2: the end-of-vote beeps
    if (CInformacaoEleitor::GetInst().m_modoAudio != 2)          // func 509: 2 = áudio desabilitado
        CVotacaoStateAudio::PlayMessage("fim");
    CThreadOperador::GetInst().EnviaMensagem({EMensagemOperador::FimVotoEleitor}, 1);
    m_proximoEstado = &CAguardaMensagem::GetInst();         // func 1337
}

}  // namespace vota
