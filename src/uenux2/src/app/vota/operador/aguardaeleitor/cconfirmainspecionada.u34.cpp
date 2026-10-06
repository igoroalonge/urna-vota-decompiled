// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/operador/aguardaeleitor/cconfirmainspecionada.cpp  (path inferred)
// Class declaration: ../estadosoperador.u34.h.
//
// Periodic inspection of the voting booth ("inspeção da cabina e da urna"). Between voters the operator
// thread draws the time of the next inspection (IInformacaoThreadOperador::SorteiaProximaInspecao, now +
// 60..90 min, func 3620). When it is due, CPedeIdentidade::ProcessTick sends MSG_ELEITOR_INSPECAO to the
// voter terminal and goes to CAguardaInspecao ("Inspecione cabina e urna"). The voter terminal shows
// CInspecionaUrna; when the mesário confirms there, the voter thread logs "Inspeção da urna confirmada" and
// posts message 11 to the operator; CAguardaInspecao::ProcessMessage(11) (func 10530) builds this state
// (constructor inlined there):
//     CAppState(2); clock {33,1}; (20,2) centred "Inspeção completa";
//     (40,4) right "CONFIRMA: continuar a votação"; control input; CriaFormInterativo("", true).
//
// WEB BUILD: dead code (operator thread not run).
#include "api/ipc/cmessagequeue.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/log/clogvota.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/operador/estadosoperador.u34.h"
#include "vota/operador/leidentidade/cpedeidentidade.h"

namespace vota {

// wasm func 10527 - vtable slot 7 (ProcessInput). srcloc cinteractiveform.h:57 @1590836.
void CConfirmaInspecionada::ProcessInput()
{
    if (m_form->Read() != api::EInputResult::CONFIRMA)
        return;

    CLogVota::GetInst().Loga("Inspeção da urna terminada");             // api_f233: severity 1 (26 chars)

    // Voter terminal: message 13 -> CUrnaInspecionada::ProcessMessage shows the "continue" screen and goes
    // back to CAguardaMensagem (u18 fragment).
    auto& fila = CThreadEleitor::GetInst().Fila();                      // func 316 + 36
    fila.Add(api::SMessage{13, &fila}, 1);                              // rhvoice_f501

    impl::IInformacaoThreadOperador::GetInst().SorteiaProximaInspecao();   // func 3620 (slot 17)
    m_proximoEstado = &CPedeIdentidade::GetInst();                      // func 652
}

}  // namespace vota
