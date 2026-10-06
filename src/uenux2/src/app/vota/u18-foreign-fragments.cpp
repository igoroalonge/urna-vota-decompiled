// FRAGMENTS reconstructed by unit u18 from vota_web_wasm.wasm - VOTA functions that the analyzer put in
// unit u18 (because of an inlined api/ipc srcloc or by call-graph proximity). Each block names its
// original file (paths inferred from the TSE convention class CFooBar -> cfoobar.cpp unless attested).
//
// Vocabulary used below (see docs/modules/u07 and u10):
//   CThreadEleitor::GetInst()   wasm 316  (the tools call it CPriorityMessageQueue<SMessage>::ctor@316)
//   CThreadOperador::GetInst()  wasm 270  (tools: ...::ctor@270)
//   <thread>.m_fila             vota::CMessageEleitor / CMessageOperador at thread+36
//   m_fila.Add(msg, 1)          rhvoice_f501 (CPriorityMessageQueue<SMessage>::Add, name inferred)
//   comum::CAppState            +4 m_proximoEstado (the state returned by GetNextState)
//   CAguardaMensagem::GetInst() wasm 1337 = vota_f764(mutex @1832940, &instance @1832964, vtable
//                               vota::CAguardaMensagem @1532964, flags 1) - lazy singleton
#include "api/ipc/cmessagequeue.h"
#include "api/ipc/cthread.h"
#include "comum/cappstate.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/monitor/cthreadmonitor.h"
#include "vota/operador/cthreadoperador.h"

namespace vota {

// wasm funcs 6018 and 6058 are most likely NOT source functions but wasm-opt merge-similar-functions
// bodies (docs/libraries/libcxx-core.md §1.2): each has exactly two callers, which are tiny thunks that differ
// only in one constant (6018: CControladorRegistraMesariosVota slots 10/15 pass 11/7; 6058:
// CReinicioComparecimentoMesario / CFinalizaAquisicao ::StartState pass 7/10). 6018 also keeps an unused first
// parameter - the `this` of the table-referenced slot methods it was merged from; a source-level helper would
// not have it (or dead-argument elimination would have removed it). So the bodies are written inline below.
//   6018(this, id): post {id, &fila} with priority 1 to CThreadEleitor::GetInst().m_fila   (316 + 36)
//   6058(this, id): m_proximoEstado = &CAguardaMensagem::GetInst() (1337); post {id, &fila}, priority 1,
//                   to CThreadOperador::GetInst().m_fila   (270 + 36)

// =============================================================================================
// uenux2/src/app/vota/eleitor/caguardamensagem.cpp (path inferred)
// wasm func 1337 (name inferred): CAguardaMensagem::GetInst() - lazy singleton (CAppState flags 1 =
// receives messages). The web build's first voter state (main -> CExecucaoVotaCooperativa, u06).
CAguardaMensagem& CAguardaMensagem::GetInst();

// =============================================================================================
// uenux2/src/app/vota/eleitor/cthreadeleitor.cpp  (attested)  - reconstructed in cthreadeleitor.cpp (u07)
// wasm func 316: CThreadEleitor::GetInst()   84-byte object, static unique_ptr @1833212, mutex @1833188.
//   new CThreadEleitor: api::CThread() (3598); tick map (+20) empty; m_pContexto (+32) null;
//   m_fila (+36) = CMessageEleitor: CPriorityMessageQueue ctor (cmessagequeue.h:113/114) + CMessageInterface.
//   Observed executing (every votaTick / key press goes through it).
//
// uenux2/src/app/vota/operador/cthreadoperador.cpp  (attested by its srcloc'ed functions)
// wasm func 270 (name inferred): CThreadOperador::GetInst()   132-byte object, unique_ptr @1911708,
// mutex @1911684. Same construction plus: +84 bool = false; +96 std::string (título do mesário being
// registered, see slots 17/18 below) = ""; +108 std::string = " " ("VOTANDO PARA: ..." text, u10);
// +120 std::string = "" (text shown by the "{:<12s}" source 10728). Observed executing: the voter
// states post to it (the queue is never read in the web build, u10 §2).
CThreadOperador& CThreadOperador::GetInst()
{
    static std::unique_ptr<CThreadOperador> s_instancia;                 // @1911708
    std::lock_guard lock(s_mutex);                                        // @1911684 (only the unlock stub remains)
    if (!s_instancia)
        s_instancia = std::make_unique<CThreadOperador>();
    return *s_instancia;
}

// =============================================================================================
// uenux2/src/app/vota/cexecucaovota.cpp (path inferred; iexecucaovota.cpp:74 is attested for the interface)
// vota::CExecucaoVota : vota::IExecucaoVota - the urna's execution policy: real threads.
// IExecucaoVota slots (names inferred): 2 Executa, 3 Inicia, 4 Aguarda, 5 IniciaOperador,
// 6 FinalizaThreads, 7 Processa (return false here), 8 GetFilaEleitor, 9 GetEstadoAtual.
// Registered only when nothing else is (main registers CExecucaoVotaCooperativa), so in the simulator these
// never run - which matters, because the sleeps below and CWasmThread::Wait call emscripten_sleep, and
// this build has no Asyncify: they would abort().

// wasm func 10236 (slot 2)
void CExecucaoVota::Executa()
{
    Inicia();                                                            // inlined copy of slot 3
    Aguarda();                                                           // inlined copy of slot 4
}

// wasm func 10235 (slot 3)
void CExecucaoVota::Inicia()
{
    CThreadEleitor::GetInst().Start();                                   // 316 -> 1684
    CThreadOperador::GetInst().Start();                                  // 270 -> 1684
    CThreadMonitor::GetInst().Start();                                   // 1898 -> 1684
    api::Dorme(1000);   // `if (g_esperaHabilitada @1584624) emscripten_sleep(1000)`        name inferred
}

// wasm func 10234 (slot 4)
void CExecucaoVota::Aguarda()
{
    CThreadEleitor::GetInst().Wait();                                    // 1900
    CThreadOperador::GetInst().Wait();
    CThreadMonitor::GetInst().Wait();                                    // 1898 -> 1900
}

// (not in u18, for reference) slot 5 = 10233: CThreadOperador::GetInst().Start();
// slot 6 = 10232: CThreadOperador::GetInst().m_bParar = true; CThreadMonitor::GetInst().m_bParar = true.

// =============================================================================================
// uenux2/mock/app/vota/cexecucaovotacooperativa.cpp (path inferred)
// vota::CExecucaoVotaCooperativa : IExecucaoVota - the web policy: no threads, votaTick drives
// CThreadEleitor::Processar() (4349) through slot 7 (7823). +4 = the first state given by main (CAguardaMensagem).
// wasm func 4713 (slots 2 AND 3: Executa and Inicia share the body; observed executing, votaInit) was
// reconstructed here by u18. It now lives with the constructor (7828) and slot 7 (7823) in
// src/uenux2/mock/app/vota/cexecucaovotacooperativa.cpp.

// =============================================================================================
// uenux2/src/app/vota/eleitor/cmostratelacontinuavotacao.cpp (path inferred)
// wasm func 7221 (slot 2 StartState). Reached from CAguardaMensagem on message 14.
void CMostraTelaContinuaVotacao::StartState()
{
    m_proximoEstado = this;
    m_tela->Mostra();                                                    // +12, slot 2 (CTelasVota::CriaTelaContinuacaoVotacao)
    auto& fila = CThreadOperador::GetInst().m_fila;
    fila.Add(api::SMessage{12, &fila}, 1);                               // tell the operator terminal
    m_proximoEstado = &CAguardaMensagem::GetInst();                      // the screen stays up
}

// =============================================================================================
// uenux2/src/app/vota/eleitor/curnainspecionada.cpp (path inferred)
// wasm func 11794 (slot 6 ProcessMessage; name from u02's slot table)
void CUrnaInspecionada::ProcessMessage(int mensagem)
{
    if (mensagem == 13) {
        m_telaContinuacao->Mostra();                                     // +20, slot 2
        m_proximoEstado = &CAguardaMensagem::GetInst();
    }
}

// =============================================================================================
// uenux2/src/app/vota/eleitor/creiniciocomparecimentomesario.cpp (path inferred)
// wasm func 11917 (slot 2 StartState): the voter terminal waits while the operator registers the mesários
// (re-start of the attendance registration, estadoVota 58 "registromesariofinal", u06).
void CReinicioComparecimentoMesario::StartState()                       // body = merged 6058 with id 7
{
    m_proximoEstado = &CAguardaMensagem::GetInst();
    auto& fila = CThreadOperador::GetInst().m_fila;
    fila.Add(api::SMessage{7, &fila}, 1);
}

// uenux2/src/app/vota/eleitor/cfinalizaaquisicao.cpp (path inferred)
// wasm func 12113 (slot 2 StartState): end of vote acquisition (estadoVota 57 "fimaquisicaovotos"): the
// voter terminal waits, the operator gets message 10 (then registers the closing mesários and finally
// sends message 7 back - see CControladorRegistraMesariosVota slot 15 - which starts the BU).
void CFinalizaAquisicao::StartState()                                   // body = merged 6058 with id 10
{
    m_proximoEstado = &CAguardaMensagem::GetInst();
    auto& fila = CThreadOperador::GetInst().m_fila;
    fila.Add(api::SMessage{10, &fila}, 1);
}

// =============================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cdefinerotaprevotacao.cpp (path inferred)
// wasm func 11994 (slot 3 NeedChangeState)
bool CDefineRotaPreVotacao::NeedChangeState()
{
    const auto informacao = comum::CConfiguracaoEleicao::GetInst().GetInformacaoEleicao();   // 603 (cfg +88)
    if (informacao.EhModoDemonstracao() && !comum::EhTreinamentoEleitor()) {                  // 1950, 697
        m_campo12 = 0;                                                                        // +12 ?
        m_proximoEstado = &CAguardaMensagem::GetInst();
        auto& fila = CThreadOperador::GetInst().m_fila;
        fila.Add(api::SMessage{7, &fila}, 1);
    } else {
        m_proximoEstado = &CIniciodeCiclo::GetInst();                                          // 4433
        m_campo12 = 0;
    }
    return true;
}

// =============================================================================================
// uenux2/src/app/vota/operador/comparecimentomesario/cregistromesarioencerrado.cpp (path inferred)
// wasm func 10763 (slot 2 StartState). Operator side, end of a mesário registration round: the voter
// terminal receives 14 -> CMostraTelaContinuaVotacao.
void CRegistroMesarioEncerrado::StartState()
{
    m_proximoEstado = this;
    auto& fila = CThreadEleitor::GetInst().m_fila;
    fila.Add(api::SMessage{14, &fila}, 1);
}

// =============================================================================================
// uenux2/src/app/vota/operador/comparecimentomesario/ccontroladorregistramesariosvota.cpp (path inferred)
// vota::CControladorRegistraMesariosVota : comum::IControladorRegistraMesarios (37 slots). u18 has four.
// Slots 10 and 15 are called by comum::CEncerraRegistroMesarios::StartState (wasm 10380, tools name
// "GetControlador"): phase (slot 6 = estadoVota - 55 via a table) 0/1 -> slot 10, phase 3 (estadoVota 58
// "registromesariofinal") -> slot 15; both after setting the next state and calling slot 29.

// wasm func 10792 (slot 10; name inferred): initial registration finished -> voter terminal message 11
// (CAguardaMensagem -> CIniciodeCiclo: the urna is ready for the first voter).
void CControladorRegistraMesariosVota::LiberaTerminalEleitor() const   // body = merged 6018 with id 11
{
    auto& fila = CThreadEleitor::GetInst().m_fila;
    fila.Add(api::SMessage{11, &fila}, 1);
}

// wasm func 10787 (slot 15; name inferred): final registration finished -> voter terminal message 7
// (CAguardaMensagem: estadoVota = GERARBU, "Inicio do Encerramento", next state CGeraBU = the BU).
void CControladorRegistraMesariosVota::IniciaEncerramento() const      // body = merged 6018 with id 7
{
    auto& fila = CThreadEleitor::GetInst().m_fila;
    fila.Add(api::SMessage{7, &fila}, 1);
}

// wasm func 10788 (slot 17; name inferred): título typed for the mesário being registered.
const std::string& CControladorRegistraMesariosVota::GetTituloDigitado() const
{
    return CThreadOperador::GetInst().m_tituloMesario;                   // +96
}

// wasm func 10785 (slot 18; name inferred)
void CControladorRegistraMesariosVota::SetTituloDigitado(const std::string& titulo) const
{
    CThreadOperador::GetInst().m_tituloMesario = titulo;                 // +96 (self-assignment guarded)
}

// =============================================================================================
// uenux2/src/app/vota/operador/.../ctituloencerramentoinvalido.cpp (path inferred from the curated evidence)
// wasm func 10728 (name inferred): text source of the CTituloEncerramentoInvalido screen (table slot 3781).
std::string TextoCampoOperador120()
{
    return std::format("{:<12s}", CThreadOperador::GetInst().m_texto120);   // +120, left-aligned, 12 columns
}

}  // namespace vota
