// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.cpp  (path inferred)
// Class declaration: ../estadosoperador.u34.h. Constructor/GetInst (func 5393): u17-foreign-fragments.cpp.
// StartState (func 10436): unit u39.
//
// Flow on the urna: the voter finished (message 1 "FimVotoEleitor" to the operator thread) and had the
// audio on (CInformacaoEleitor::m_modoAudio != 2) -> CSincronismoOperador / CEleitorVotouNaoVotou go to
// this state: the MT beeps and asks the mesário to take the headphones off the urna. CONFIRMA switches the
// voter terminal's audio off and returns to the identification screen.
//
// WEB BUILD: dead code (the operator thread never runs). If it ran in the browser, the wait loop below
// would never end: usleep() is a busy loop on performance.now() (func 6265) and the voter thread, which is
// the only code that can set m_modoAudio = 2, is stepped by the same JavaScript thread.
#include <unistd.h>

#include "api/ipc/cmessagequeue.h"
#include "vota/eleitor/comum/cinformacaoeleitor.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/log/clogvota.h"
#include "vota/operador/estadosoperador.u34.h"
#include "vota/operador/leidentidade/cpedeidentidade.h"

namespace vota {

// wasm func 10435 - vtable slot 7 (ProcessInput). srcloc cinteractiveform.h:57 @1592912 (Read inlined).
void CDesabilitaAudioEleitor::ProcessInput()
{
    if (m_form->Read() != api::EInputResult::CONFIRMA)                          // 9
        return;

    // Tell the voter thread to turn its audio off: message 10 (MSG_AUDIO_DESABILITADO), priority 1.
    auto& fila = CThreadEleitor::GetInst().Fila();                              // func 316, queue at +36
    fila.Add(api::SMessage{CThreadEleitor::MSG_AUDIO_DESABILITADO, &fila}, 1);  // rhvoice_f501 (misnamed Add)

    // Wait until the voter thread has processed it (CInformacaoEleitor +4 = 2 "sem áudio"). No timeout.
    while (CInformacaoEleitor::GetInst().m_modoAudio != 2)                      // func 509
        usleep(300);                                                            // func 3966 (300 µs)

    CLogVota::GetInst().LogaAudioDesativadoFimVotacao();    // func 4529 "Áudio desativado pelo fim da votação"
    m_proximoEstado = &CPedeIdentidade::GetInst();          // func 652: back to "Digite o Título ou o CPF"
}

}  // namespace vota
