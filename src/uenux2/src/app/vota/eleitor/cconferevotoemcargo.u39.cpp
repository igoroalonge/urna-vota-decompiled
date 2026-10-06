// FRAGMENT reconstructed by unit u39 from vota_web_wasm.wasm.
// Original file (attested by srclocs :41/:80/:95 of sibling functions): uenux2/src/app/vota/eleitor/cconferevotoemcargo.cpp
// Class declaration: cconferevotoemcargo.h (unit u06). The rest of the file: cconferevotoemcargo.cpp (u06).
//
// vota::IConfereVotoEmCargo = the transient "conferência" screen of a vote ("Confira o seu voto"), common
// base of the 10 CConfereVotoEmCargo<TConfirma, ETelaVotacao> instances. These four slots are shared by all
// instances (the template instances only override slot 16 GetProximoEstado).
#include "vota/eleitor/cconferevotoemcargo.h"

#include "api/audio/cesperaaudio.h"
#include "vota/eleitor/cthreadeleitor.h"

namespace vota {

// wasm func 1717 - vtable slot 0 (IConfereVotoEmCargo and the 10 template instances; slot 1 = ICF 325 /
// func 1278 + operator delete). Cancels a pending "run after the current audio" wait, then the members and
// the CVotacaoStateAudio base (func 1035). The cancel sequence is FinishStateAudio (func 11792) inlined.
IConfereVotoEmCargo::~IConfereVotoEmCargo()
{
    if (m_espera) {
        m_espera->Cancela();                   // CEsperaAudio slot 2 (name inferred)
        m_espera.reset();
    }
}

// wasm func 11792 - vtable slot 11 (FinishStateAudio): leaving the conferência cancels the pending wait,
// so a late "audio finished" callback cannot switch the state.
void IConfereVotoEmCargo::FinishStateAudio()
{
    if (m_espera) {
        m_espera->Cancela();
        m_espera.reset();
    }
}

// wasm func 11790 - vtable slot 12 (ProcessTickAudio). Observed executing: this is how every conferência of
// the recorded votes ended. When the conferência tick (+28, started by StartStateAudio when m_comTick) fires,
// stop it and switch to the confirmation state (CONFIRMA/CORRIGE screen) as soon as the current audio
// message has finished. Until then every key is refused ("Tecla indevida pressionada", ProcessInputAudio).
void IConfereVotoEmCargo::ProcessTickAudio(uebyte tick)
{
    if (!m_comTick || tick != m_tick)                                  // +29, +28
        return;
    CThreadEleitor::GetInst().StopTick(m_tick);                        // func 316 -> 422
    ExecutarAposAudioAtual([this] { m_proximoEstado = GetProximoEstado(); });   // func 3851;
                                                                       // lambda ProcessTickAudio(unsigned char)::$_0,
                                                                       // std::function vtable @1547848
}

// wasm func 11789 - vtable slot 15. Literal @362811 (19 characters, copied into a 24-byte heap buffer).
std::string IConfereVotoEmCargo::GetMensagemAudio() const
{
    return "Confira o seu voto.";
}

}  // namespace vota
