// Reconstructed from vota_web_wasm.wasm (unit u07). Original: uenux2/src/app/vota/eleitor/cthreadeleitor.cpp
// (attested: std::source_location record @1534648, line 134, inside wasm func 4349).
//
// Functions of this file found in the binary:
//   2072  LogDebug                      (static; thunk into the merged body 2921)
//   2921  LogSistema                    (header helper, merged by wasm-opt with 3 other translation units)
//   2438  CThreadEleitor::~CThreadEleitor            vtable slot 0
//   7070  CThreadEleitor deleting destructor        vtable slot 1
//   7061  CThreadEleitor::Run                       vtable slot 2
//   7030  CThreadEleitor::FinalizaExecucao          vtable slot 5   (name inferred)
//   4349  CThreadEleitor::Processar                 (name inferred; contains ProcessarMensagens,
//                                                    ProcessarEntrada (srcloc :134) and ProcessarTicks)
//   7122  atexit destructor of the static instance pointer
//    316  CThreadEleitor::GetInst (unit u18, reproduced here because it belongs to this file)
//
// Conventions: "// wasm func N" = function index in vota_web_wasm.wasm; "// ?" = guess;
// "// name inferred" = no symbol/srcloc evidence for the name.
#include "vota/eleitor/cthreadeleitor.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <syslog.h>
#include <typeinfo>
#include <unistd.h>
#include <vector>

#include "api/gui/iinputkbd.h"                          // api::IInputKbd (poly-singleton)
#include "api/util/csystem.h"                           // api::CSystem::Sleep (see Run)
#include "vota/iexecucaovota.h"                     // vota::IExecucaoVota (iexecucaovota.cpp:74)
#include "vota/eleitor/cajusteinicial.h"            // vota::IAjusteInicial / CAjusteInicial
#include "vota/eleitor/comum/cinformacaoeleitor.h"  // vota::CInformacaoEleitor (wasm func 509)

namespace vota {

namespace {

// ---------------------------------------------------------------------------------------------------
// Logging helper. It comes from a shared header and has internal linkage, so every translation unit
// owns a copy with its own "am I on a real urna?" cache. wasm-opt's merge-similar-functions folded the
// copies of cthreadeleitor.cpp (2072 -> cache @1534908), cthreadoperador.cpp (3593 -> @1601364),
// cvalidamidia.cpp (5574 -> @1577092) and one more caller (5895 -> @1552472) into one body, wasm func
// 2921, which receives the priority and the cache address as extra parameters. The caches start at -1.
//
// wasm func 2921 (merged body; original header path unknown)                       // name inferred
void LogSistema(int prioridade, int& urnaReal, const char* formato, va_list args)
{
    if (urnaReal == -1)
        urnaReal = (access("/dev/urna", F_OK) == 0);

    if (urnaReal) {
        vsyslog(prioridade, formato, args);
    } else if (std::getenv("DEBUG_UENUX") != nullptr) {
        // ? A local buffer (140 bytes, sp+16..sp+155 of the 160-byte frame) is printed as the line prefix, but
        //   nothing in this build writes it: the call that fills it on Linux (probably a thread/process-name
        //   query) was compiled away. clang folded printf("%s: ", ...) into iprintf with the constant "%s: ".
        char prefixo[140];
        std::printf("%s: ", prefixo);
        std::vprintf(formato, args);
    }
}

int s_urnaReal = -1;                                     // @1534908

// wasm func 2072                                                                    // name inferred
void LogDebug(const char* formato, ...)
{
    va_list args;
    va_start(args, formato);
    LogSistema(LOG_DEBUG /* 7 */, s_urnaReal, formato, args);
    va_end(args);
}

// Name of the dynamic type of the current state, for the debug log ("" when there is no state).
// (inlined everywhere: typeinfo = vtable[-1], name = typeinfo+4; "" is the tail-merged NUL @450187)
const char* NomeEstado(const comum::CAppStateContext& contexto)
{
    const comum::CAppState* estado = contexto.GetEstado();
    return estado ? typeid(*estado).name() : "";
}

} // namespace

// ---------------------------------------------------------------------------------------------------
// Singleton (wasm func 316, unit u18 - kept here because it is this file's code). Constructor inlined:
//   api::CThread::CThread()                    (func 3598: m_estado = 0, m_bParar = false, m_pImpl from
//                                               IGenericFactory<IThreadImpl>, cthread.cpp:31/32)
//   CThreadVota: m_ticks = {}, m_pContexto = nullptr
//   CMessageEleitor: CPriorityMessageQueue<SMessage>() (cmessagequeue.h:113 semaphore, :114 lock)
static std::unique_ptr<CThreadEleitor> s_pInstancia;   // @1833212
static std::mutex s_mutexInstancia;                     // @1833188

CThreadEleitor& CThreadEleitor::GetInst()              // wasm func 316  // name inferred
{
    std::lock_guard<std::mutex> lock(s_mutexInstancia);
    if (!s_pInstancia)
        s_pInstancia.reset(new CThreadEleitor());
    return *s_pInstancia;
}
// wasm func 7122: the atexit handler registered for s_pInstancia (std::unique_ptr destructor):
//   s_pInstancia.reset();   -> ~CThreadEleitor (2438) + free

// ---------------------------------------------------------------------------------------------------
// wasm func 2438 (vtable slot 0); wasm func 7070 is the deleting destructor (slot 1: dtor + free).
// Everything is member destruction, inlined:
//   ~CMessageEleitor -> ~CPriorityMessageQueue: m_pLock.reset(), m_pSemaforo.reset(), vector freed
//   ~CThreadVota     -> m_pContexto.reset(), m_ticks tree destroyed (func 2125)
//   ~api::CThread    -> func 2721 (m_pSync.reset(), m_pImpl.reset())
CThreadEleitor::~CThreadEleitor() = default;

// ---------------------------------------------------------------------------------------------------
// wasm func 7061 (vtable slot 2). Thread body.
void CThreadEleitor::Run()
{
    // IAjusteInicial::GetInst() (cajusteinicial.cpp:57), inlined here:
    //   auto& lista = api::GetPolySingletonsInfo();                  // function pointer @1526320 -> func 11265
    //   if (!api::CPolySingletonList::contains<IAjusteInicial>(lista))            // func 2450
    //       api::CPolySingletonList::push<IAjusteInicial>(std::make_unique<CAjusteInicial>(), lista);
    //           // cpolysingletonlist.h:129 - throws "{}: instância já criada de {}" (6756) if present,
    //           // logs "sz[{}] ptr[{}]" when the list's debug flag (+12) is set
    //   return api::CPolySingleton<IAjusteInicial>::instance(lista, srcloc :57);  // cpolysingleton.h:78,
    //           // cpolysingletonlist.h:99/105 ("solicitada uma instância não criada", "não corresponde")
    // CAjusteInicial is a 12-byte comum::CAppState with flags 0 (vtable @1534244).
    comum::CAppState& estadoInicial = IAjusteInicial::GetInst();

    // CThreadVota::DefineEstadoInicial (inlined; name inferred): new context, then StartState of its state.
    m_pContexto = std::make_unique<comum::CAppStateContext>(&estadoInicial);
    if (comum::CAppState* estado = m_pContexto->GetEstado())
        estado->StartState();

    while (!m_bParar) {
        const bool processou = Processar();
        if (m_bParar)
            break;
        m_pImpl->Yield();                                // IThreadImpl slot 5 (CWasmThread::Yield, func 9651, which
                                                         //   itself calls emscripten_sleep(0): the first cycle
                                                         //   would already ABORT in this build, no Asyncify)
        if (!processou)
            api::CSystem::Sleep(50);                     // ? inlined as: if (flag@1584624 & 1) emscripten_sleep(50);
                                                         //   the flag is 1 in the data segment and never written.
    }
}

// ---------------------------------------------------------------------------------------------------
// wasm func 7030 (vtable slot 5)                                                    // name inferred
// IExecucaoVota slot 6: vota::CExecucaoVota (func 10232) sets m_bParar on CThreadOperador and on
// CThreadMonitor (func 1898); vota::CExecucaoVotaCooperativa (web build) has a no-op there.
void CThreadEleitor::FinalizaExecucao()
{
    IExecucaoVota::GetInst().FinalizaThreads();         // iexecucaovota.cpp:74 (func 3594); name inferred
}

// ---------------------------------------------------------------------------------------------------
// wasm func 4349                                                                    // name inferred
// Called by Run() and, in the web build, by vota::CExecucaoVotaCooperativa slot 7 (func 7823) from
// votaTick. The caller repeats the "context exists and not stopped" test before calling.
bool CThreadEleitor::Processar()
{
    if (!m_pContexto || m_bParar)
        return false;

    bool processou = ProcessarMensagens(*m_pContexto);
    if (m_bParar)
        return processou;

    bool entrada = ProcessarEntrada(*m_pContexto);
    if (!m_bParar && ProcessarTicks(*m_pContexto))
        entrada = true;

    return processou || entrada;
}

// inlined into wasm func 4349 (log strings name it: "ProcessarMensagens")
bool CThreadEleitor::ProcessarMensagens(comum::CAppStateContext& contexto)
{
    bool processou = false;

    // m_fila.Vazia(): lock (ISyncCtl slot 2), begin == end, unlock (slot 3)
    while (!m_fila.Vazia()) {
        const api::SMessage mensagem = m_fila.Remove();             // func 2073 (cmessagequeue.h:153)
        const char* nomeEstado = NomeEstado(contexto);              // taken before the message is handled

        switch (mensagem.id) {
        case MSG_TERMINA_THREAD:
        case MSG_DESLIGA:
            m_bParar = true;
            break;
        case MSG_AUDIO_CADASTRO:
            CInformacaoEleitor::GetInst().HabilitaAudioConformeCadastro();   // func 6745: m_modoAudio = 0
            break;
        case MSG_AUDIO_HABILITADO:
            CInformacaoEleitor::GetInst().HabilitaAudio();                   // func 6743: m_modoAudio = 1
            break;
        case MSG_AUDIO_DESABILITADO:
            CInformacaoEleitor::GetInst().DesabilitaAudio();                 // func 4195: m_modoAudio = 2
            break;
        default:
            LogDebug("%s: 2 msg[%d] context[%s]", "ProcessarMensagens", mensagem.id, nomeEstado);
            if (contexto.AceitaMensagens()) {                   // state && state->m_bRecebeMensagens (+8), func 3842
                contexto.ProcessMessage(static_cast<uebyte>(mensagem.id));   // state->ProcessMessage, func 3843
                contexto.VerificaTrocaEstado();                 // see below
            }
            processou = true;
            continue;
        }

        LogDebug("%s: 1 msg[%d] context[%s]", "ProcessarMensagens", mensagem.id, nomeEstado);
        if (m_bParar)
            return processou;
        processou = true;
    }
    return processou;
}

// inlined into wasm func 4349; srcloc cthreadeleitor.cpp:134 is the IInputKbd::GetInst() call
bool CThreadEleitor::ProcessarEntrada(comum::CAppStateContext& contexto)
{
    api::IInputKbd& teclado = api::IInputKbd::GetInst();           // func 455, cthreadeleitor.cpp:134

    if (!contexto.AceitaTeclado() || !teclado.HasKey())            // m_bRecebeTeclado (+9), func 5909; IInputKbd slot 3
        return false;

    LogDebug("%s: hasKey[%d] context[%s]", "ProcessarEntrada", teclado.HasKey(), NomeEstado(contexto));
    contexto.ProcessInput();                                         // state->ProcessInput (slot 7), func 5911
    contexto.VerificaTrocaEstado();
    return true;
}

// inlined into wasm func 4349                                                      // name inferred
bool CThreadEleitor::ProcessarTicks(comum::CAppStateContext& contexto)
{
    bool processou = false;
    const std::vector<uebyte> expirados = GetTicksExpirados();      // CThreadVota, func 5450 (gettimeofday)
    if (!expirados.empty() && contexto.AceitaTicks()) {             // m_bRecebeTicks (+10), func 5908
        for (const uebyte tick : expirados)
            contexto.ProcessTick(tick);                              // state->ProcessTick (slot 8), func 5910
        processou = true;
    }
    contexto.VerificaTrocaEstado();
    return processou;
}

// comum::CAppStateContext::VerificaTrocaEstado() is inlined at every use above (name inferred):
//     if (m_pEstado && m_pEstado->NeedChangeState()) {        // slot 3 (= GetNextState() != this)
//         m_pEstado->FinishState();                           // slot 5
//         m_pEstado = m_pEstado->GetNextState();              // slot 4
//         if (m_pEstado) m_pEstado->StartState();             // slot 2
//     }
// States are singletons: the old state is not deleted.

} // namespace vota
