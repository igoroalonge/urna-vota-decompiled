// Reconstructed from vota_web_wasm.wasm (unit u27). Original (path inferred from cthreadoperador.cpp,
// which is attested by the std::source_location record cthreadoperador.cpp:87 inside Run()):
// uenux2/src/app/vota/operador/cthreadoperador.h
//
// CThreadOperador is the "operator thread" of the VOTA application: the thread that owns the poll
// worker's terminal (terminal do mesário / micro-terminal, "MT": 4x40 LCD, numeric keypad, LED, buzzer,
// fingerprint sensor) and runs the operator-side state machine (comum::CAppStateContext whose states are
// CAguardaInicio, CPedeIdentidade, CEscolheOpcao, CNomeEleitor, CMostraEleitorVotando, ...).
// It talks to the voter thread (CThreadEleitor) only through the two message queues.
//
// RTTI:  api::CThread <- vota::CThreadVota <- vota::CThreadOperador      (typeinfo @1601248)
//        vtable @1601208: [0] ~CThreadOperador (2717)  [1] deleting dtor (10206)  [2] Run (10204)
//                         [3] CThreadVota::TrataExcecao (7710)  [4] TrataExcecaoDesconhecida (7709)
//                         [5] FinalizaExecucao (10203)
//        member queue: vota::CMessageOperador : api::CPriorityMessageQueue<api::SMessage> (vptr @1601268),
//                                               api::CMessageInterface (vptr @1601288)
//
// WEB BUILD: the thread is never started. main() registers vota::CExecucaoVotaCooperativa as
// IExecucaoVota and votaTick() only steps CThreadEleitor. Run() below is dead code in the simulator
// (see docs/modules/u27-uenux2-src-app-vota-operador.md §2). The voter states still post their
// operator messages; they stay unread in m_fila.
#pragma once

#include <string>

#include "api/ipc/cmessagequeue.h"     // api::CPriorityMessageQueue<api::SMessage>, api::CMessageInterface
#include "vota/cthreadvota.h"          // vota::CThreadVota (path inferred)

namespace vota {

/// vota::CMessageOperador (48 bytes, at CThreadOperador +36). Same shape as CMessageEleitor
/// (src/uenux2/src/app/vota/eleitor/cthreadeleitor.h). Constructor inlined in func 270.
class CMessageOperador : public api::CPriorityMessageQueue<api::SMessage>,   // +36 vptr @1601268
                         public api::CMessageInterface {                        // +68 vptr @1601288
    // +40 vector<SMessage> (begin/end/cap)   +56 unique_ptr<ISemaphore> (cmessagequeue.h:113)
    // +60 unique_ptr<ISyncCtl> (cmessagequeue.h:114)   +64 int = 0   +72 short = 0, +76 8 bytes = 0
};

/// Message ids CThreadOperador::Run consumes itself; every other id goes to the current state's
/// ProcessMessage (slot 6). Names inferred.
enum EMensagemThreadOperador : short {
    MSG_URNA_INOPERANTE = 0,    // shows "URNA ELETRÔNICA INOPERANTE" and stops the thread
                                // (posted with priority 100 by CVerificaEleicaoPassou::StartState, func 11914)
    MSG_TERMINA_OPERADOR = 16,  // stops the thread without touching the screen (sender not identified) ?
    // Ids forwarded to the states (see the u10/u27 docs): 1 fim do voto, 2 eleitor não votou,
    // 3/4 eleitor demorando, 5 eleitor voltou a digitar, 6 eleitor iniciou votação, 8 início da votação
    // (CAguardaInicio), 9 atualiza cargo, 11 (registro de mesários), 12/13 ...
};

class CThreadOperador : public CThreadVota {
public:
    /// wasm func 270 (unit u18; the tools call it CPriorityMessageQueue<SMessage>::CPriorityMessageQueue@270
    /// because the queue constructor and its srclocs are inlined). Lazy singleton: static unique_ptr
    /// @1911708 guarded by the mutex @1911684; 132 bytes. Reset at exit by wasm func 10208.
    static CThreadOperador& GetInst();                                     // name inferred

    ~CThreadOperador() override;                                            // slot 0 (2717) / slot 1 (10206)

    /// slot 2 (wasm func 10204, srcloc cthreadoperador.cpp:87).
    void Run() override;

    /// slot 5 (wasm func 10203). Called by CThreadVota's exception handlers (slots 3/4) before a fatal
    /// error is logged: stops the voter thread and the power monitor thread.                name inferred
    void FinalizaExecucao() override;

    CMessageOperador& GetFila() { return m_fila; }

    // Shared scratch strings written by operator states (names inferred from their users):
    std::string m_anoNascimentoDigitado;   // +84  CPedeAnoNascimento (justificativa, func 10590)
    std::string m_tituloMesario;           // +96  título typed in the mesário registration (u22, slots 17/18)
    std::string m_textoCargo = " ";        // +108 "VOTANDO PARA: <cargo>" (voter thread, message 9; u10)
    std::string m_tituloEncerramento;      // +120 título typed by the presidente to close the vote (u17)

private:
    // ---- layout (132 bytes, operator new(132) in func 270) ------------------------------------------
    // api::CThread:     +0 vptr, +4 m_estado, +8 bool m_bParar, +9 bool m_bDormindo,
    //                   +12 unique_ptr<IThreadImpl> m_pImpl, +16 unique_ptr<ISyncCtl> m_pSync
    // vota::CThreadVota: +20 std::map<uebyte, STick> m_ticks, +32 unique_ptr<CAppStateContext> m_pContexto
    CMessageOperador m_fila;               // +36 .. +83
    // +84 / +96 / +108 / +120 the four strings above
};

}  // namespace vota
