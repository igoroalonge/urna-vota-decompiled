// Reconstructed from vota_web_wasm.wasm (unit u07). Original: uenux2/src/app/vota/eleitor/cthreadeleitor.h
// (path inferred from cthreadeleitor.cpp, which is attested by a std::source_location record, line 134).
//
// CThreadEleitor is the "voter thread" of the VOTA application: the thread that owns the voter
// terminal (terminal do eleitor: screen, keyboard, audio) and runs the voter-side state machine
// (comum::CAppStateContext whose states are vota::CAjusteInicial, CEleitorVotando, CSincronismoEleitor,
// CFimVotoEleitor, the zerésima states ...). The other threads (CThreadOperador = the mesário's
// terminal, CThreadMonitor = power/headphone/flash monitor) talk to it through its message queue
// (vota::CMessageEleitor).
//
// RTTI:  api::CThread <- vota::CThreadVota <- vota::CThreadEleitor      (typeinfo @1534664)
//        vtable @1534624: [0] ~CThreadEleitor (2438)  [1] deleting dtor (7070)  [2] Run (7061)
//                         [3] CThreadVota::TrataExcecao (7710)  [4] CThreadVota::TrataExcecaoDesconhecida (7709)
//                         [5] FinalizaExecucao (7030)
//        The member queue is  vota::CMessageEleitor : api::CPriorityMessageQueue<api::SMessage>,
//                                                     api::CMessageInterface   (typeinfo @1534716, vmi)
//
// In the web build the thread is never started: main() registers vota::CExecucaoVotaCooperativa
// as IExecucaoVota, whose slot 7 (func 7823) calls CThreadEleitor::Processar() once per votaTick.
#pragma once

#include <cstdint>
#include <map>
#include <memory>

#include "api/ipc/cthread.h"          // api::CThread, api::IThreadImpl, api::ISyncCtl (path attested: api/ipc/cthread.cpp)
#include "api/ipc/cmessagequeue.h"    // api::CPriorityMessageQueue<api::SMessage>, api::SMessage
#include "vota/cthreadvota.h"     // vota::CThreadVota (path inferred)
#include "comum/cappstate.h"          // comum::CAppState, comum::CAppStateContext (path inferred)

namespace vota {

using uebyte = std::uint8_t;

/// Message ids the voter thread consumes itself in ProcessarMensagens (switch in wasm func 4349).
/// Every other id is forwarded to the current state (CAppState::ProcessMessage).
/// Senders: votaInit (1, 9), votaTick (5), operator states CHabilitaAudioEleitor (8 / 9, then 1)
/// and CDesabilitaAudioEleitor (10). Enumerator names inferred.
enum EMensagemThreadEleitor : short {
    MSG_TERMINA_THREAD        = 0,    // stops the thread (m_bParar = true)
    MSG_INICIA_ELEITOR        = 1,    // forwarded to the state (CAguardaMensagem -> voter enabled)  ?
    MSG_SINCRONIZA_VOTO       = 5,    // forwarded to CSincronismoEleitor (votaTick in the web build)
    MSG_DESLIGA               = 6,    // stops the thread (m_bParar = true)                            ?
    MSG_AUDIO_CADASTRO        = 8,    // CInformacaoEleitor::m_modoAudio = 0 ("Áudio ativado conforme cadastro")
    MSG_AUDIO_HABILITADO      = 9,    // CInformacaoEleitor::m_modoAudio = 1
    MSG_AUDIO_DESABILITADO    = 10,   // CInformacaoEleitor::m_modoAudio = 2 (CDesabilitaAudioEleitor)
};

/// vota::CMessageEleitor (48 bytes, lives at CThreadEleitor+36). Constructor inlined in wasm func 316.
/// api::CPriorityMessageQueue<api::SMessage> ctor (cmessagequeue.h:113/114) obtains its semaphore
/// and lock from the IGenericFactory<ISemaphore> / IGenericFactory<ISyncCtl> poly-singletons.
class CMessageEleitor : public api::CPriorityMessageQueue<api::SMessage>,   // +0  vptr @1534684
                        public api::CMessageInterface {                        // +32 vptr @1534704
    // CPriorityMessageQueue part:
    //   +4  std::vector<api::SMessage> m_mensagens   (begin +4, end +8, cap +12)
    //   +16 ?                                         (not initialised by the ctor)
    //   +20 std::unique_ptr<api::ISemaphore> m_pSemaforo
    //   +24 std::unique_ptr<api::ISyncCtl>   m_pLock    (slot 2 = lock, slot 3 = unlock)
    //   +28 int ? = 0
    // CMessageInterface part: +32 vptr, +36 short = 0, +40 8 bytes = 0
};

class CThreadEleitor : public CThreadVota {
public:
    /// wasm func 316 (unit u18; the tools mis-named it CPriorityMessageQueue<SMessage>::ctor because the
    /// queue constructor and its source_location records are inlined). Lazy singleton kept in a static
    /// std::unique_ptr (@1833212, guarded by a mutex @1833188 that the single-threaded build reduces
    /// to an unlock stub). Destroyed at exit by wasm func 7122.
    static CThreadEleitor& GetInst();                                      // name inferred

    ~CThreadEleitor() override;                                             // slot 0: func 2438, slot 1: func 7070

    /// slot 2 (func 7061). The thread body: installs the initial state (IAjusteInicial) and loops
    /// Processar() until m_bParar. Unreachable in the web build (emscripten_sleep would abort).
    void Run() override;

    /// slot 5 (func 7030). Called by CThreadVota's exception handlers (slots 3/4) before logging a
    /// fatal error: asks the execution policy to stop the other threads.
    void FinalizaExecucao() override;                                       // name inferred

    /// wasm func 4349 (outer function; name inferred). One "cycle" of the voter thread:
    /// messages, then keyboard, then timers. Returns true when anything was processed.
    /// Formerly shown by the tools as ProcessarEntrada, because the srcloc record of the inlined
    /// ProcessarEntrada(comum::CAppStateContext&) (line 134) is the only one inside it.
    bool Processar();

    CMessageEleitor& GetFila() { return m_fila; }                          // (IExecucaoVota slot 8 returns &m_fila)

    // CThreadVota API used by the states (defined in cthreadvota.cpp, other units):
    //   uebyte CriaTick(unsigned ms); void StartTick(uebyte); void StopTick(uebyte); ...

private:
    bool ProcessarMensagens(comum::CAppStateContext& contexto);   // inlined into 4349 (log text "ProcessarMensagens")
    bool ProcessarEntrada(comum::CAppStateContext& contexto);     // inlined into 4349 (srcloc cthreadeleitor.cpp:134)
    bool ProcessarTicks(comum::CAppStateContext& contexto);       // inlined into 4349 (name inferred)

    // ---- layout (84 bytes, operator new(84) in func 316) --------------------------------------------
    // api::CThread:
    //   +0  vptr
    //   +4  int  m_estado            (1 = started; set by CThread::Start, func 1684)
    //   +8  bool m_bParar            ("stop requested"; tested everywhere as a[8]:ubyte)
    //   +9  bool m_bDormindo         (used by CThreadOperador::Run around usleep)
    //   +12 std::unique_ptr<api::IThreadImpl> m_pImpl   (simulador::CWasmThread in the web build;
    //                                                    slot 2 Create, slot 5 Yield)
    //   +16 std::unique_ptr<api::ISyncCtl>    m_pSync
    // vota::CThreadVota:
    //   +20 std::map<uebyte, STick> m_ticks             (begin +20, root +24, size +28; func 5450 lists
    //                                                    the expired ones, func 2125 destroys the tree)
    //   +32 std::unique_ptr<comum::CAppStateContext> m_pContexto
    // vota::CThreadEleitor:
    CMessageEleitor m_fila;                                          // +36 .. +83
};

} // namespace vota
