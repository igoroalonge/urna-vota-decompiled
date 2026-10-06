// FRAGMENT reconstructed by unit u39 from vota_web_wasm.wasm.
// Original file (path inferred by u17): uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.cpp
// Class declaration: vota/operador/estadosoperador.u34.h (unit u34). Constructor/GetInst (func 5393):
// operador/u17-foreign-fragments.cpp. ProcessInput (func 10435, unit u34): on CONFIRMA posts message 10 to the
// voter thread (CInformacaoEleitor::m_modoAudio = 2), busy-waits with usleep(300) until the voter thread has
// applied it, logs "Áudio desativado pelo fim da votação" and returns to CPedeIdentidade.
//
// "Retire o fone de ouvido da urna / CONFIRMA": shown after a voter who voted with headphones finished.
// RTTI: comum::CAppState <- vota::CDesabilitaAudioEleitor (typeinfo @1592900, vtable @1592864)
//   [2] StartState 10436 (this file)  [7] ProcessInput 10435 (u34)
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/log/clogvota.h"
#include "vota/operador/estadosoperador.u34.h"

namespace vota {

// wasm func 10436 - vtable slot 2
void CDesabilitaAudioEleitor::StartState()
{
    CLogVota::GetInst().Loga("Solicitado ao mesário que desconecte o fone de ouvido");   // api_f233 (severity 1)
    m_form->Show();                                                                      // +12, IForm slot 2
    m_proximoEstado = this;
}

}  // namespace vota
