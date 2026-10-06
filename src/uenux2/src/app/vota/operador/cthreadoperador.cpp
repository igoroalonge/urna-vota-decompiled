// Reconstructed from vota_web_wasm.wasm (unit u27). Original: uenux2/src/app/vota/operador/cthreadoperador.cpp
// (attested: std::source_location record @1601232, cthreadoperador.cpp:87, inside Run()).
//
// Functions of this file found in the binary:
//   2717   CThreadOperador::~CThreadOperador              vtable slot 0
//   10206  CThreadOperador deleting destructor            vtable slot 1
//   10204  CThreadOperador::Run                           vtable slot 2  (srcloc :87)
//   10203  CThreadOperador::FinalizaExecucao              vtable slot 5  (name inferred)
//   3593   LogDebug (static; thunk into the merged body 2921, cache @1601364)
//   10208  atexit destructor of the static instance pointer (@1911708)
//   270    CThreadOperador::GetInst (unit u18; summarised here because it belongs to this file)
// Also attributed to this file by the tools, but belonging elsewhere:
//   5343   CAguardaInicio::GetInst (constructor inlined) - see u27-foreign-fragments.cpp
//
// Conventions: "// wasm func N" = function index; "// ?" = guess; "// name inferred" = no symbol evidence.
#include "vota/operador/cthreadoperador.h"

#include <cstdarg>
#include <memory>
#include <mutex>
#include <syslog.h>
#include <typeinfo>
#include <unistd.h>

#include "api/gui/cformbuildermt.h"          // api::CFormBuilderMT (template instances 180/1694, see u17)
#include "api/hwil/iinput.h"                  // api::IInputMT
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/monitor/cthreadmonitor.h"          // vota::CThreadMonitor::GetInst (func 1898)
#include "vota/operador/caguardainicio.h"     // path inferred

namespace vota {

namespace {

// wasm func 3593 (tools: vota_f3593). Same header helper as cthreadeleitor.cpp's LogDebug: wasm-opt
// merged the copies of four translation units into func 2921 (priority + "is this a real urna?" cache
// as extra parameters). This copy's cache is @1601364 (initialised to -1).       name inferred
int s_urnaReal = -1;                                                          // @1601364

void LogDebug(const char* formato, ...)
{
    va_list args;
    va_start(args, formato);
    LogSistema(LOG_DEBUG /* 7 */, s_urnaReal, formato, args);   // func 2921: vsyslog on /dev/urna,
                                                                //   vprintf when DEBUG_UENUX is set
    va_end(args);
}

const char* NomeEstado(const comum::CAppStateContext* contexto)
{
    const comum::CAppState* estado = contexto ? contexto->GetEstado() : nullptr;
    return estado ? typeid(*estado).name() : "";               // "" = the tail-merged NUL @450187
}

// comum::CAppStateContext::VerificaTrocaEstado(), inlined 3x in Run():
//   if (estado && estado->NeedChangeState()) { estado->FinishState();          // slots 3, 5
//        estado = estado->GetNextState(); if (estado) estado->StartState(); }  // slots 4, 2

}  // namespace

// ---------------------------------------------------------------------------------------------------
// wasm func 270 (unit u18) - GetInst with the constructor inlined:
//   api::CThread::CThread() (func 3598); CThreadVota: m_ticks = {}, m_pContexto = nullptr;
//   CMessageOperador(): semaphore from IGenericFactory<ISemaphore> (cmessagequeue.h:113, func 4356),
//                       lock from IGenericFactory<ISyncCtl> (:114, func 3164);
//   m_anoNascimentoDigitado = "", m_tituloMesario = "", m_textoCargo = " ", m_tituloEncerramento = "".
// wasm func 10208: atexit handler of the static unique_ptr (s_pInstancia.reset()).

// ---------------------------------------------------------------------------------------------------
// wasm func 2717 (vtable slot 0); wasm func 10206 = deleting destructor (slot 1).
// Everything is member destruction, inlined: the four strings (+120, +108, +96, +84), ~CMessageOperador
// (lock and semaphore unique_ptrs, message vector), ~CThreadVota (m_pContexto, tick tree func 2125),
// then api::CThread::~CThread (func 2721).
CThreadOperador::~CThreadOperador() = default;

// ---------------------------------------------------------------------------------------------------
// wasm func 10203 (vtable slot 5)                                                    name inferred
void CThreadOperador::FinalizaExecucao()
{
    CThreadEleitor::GetInst().m_bParar = true;                        // func 316, +8
    CThreadMonitor::GetInst().m_bParar = true;                        // func 1898, +8
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10204 (vtable slot 2, srcloc :87). The body of the operator thread. Compared with
// CThreadEleitor::Processar (func 4349) the three phases are written out in the loop, the key test uses
// the MT keypad, and an idle cycle sleeps with usleep(50 ms) while flagging m_bDormindo.
//
// WEB BUILD: never called (only the harness tools/bu/operator_harness.mjs runs a patched copy).
void CThreadOperador::Run()
{
    // Initial state: CAguardaInicio (func 5343), then its StartState.
    m_pContexto = std::make_unique<comum::CAppStateContext>(&CAguardaInicio::GetInst());   // func 3844
    if (comum::CAppState* estado = m_pContexto->GetEstado())
        estado->StartState();

    api::IInputMT& teclado = api::CPolySingletonList::instance<api::IInputMT>();          // line 87 (func 383)

    while (!m_bParar) {
        bool processou = false;

        // ---- 1. messages -------------------------------------------------------------------------
        while (!m_fila.Vazia()) {                                   // lock (ISyncCtl slot 2), begin != end, unlock
            processou = true;
            const api::SMessage mensagem = m_fila.Remove();         // func 2073 (cmessagequeue.h:153)
            LogDebug("%s: msg[%d]", "MensagemProcessadaPelaThread", mensagem.id);

            if (mensagem.id == MSG_URNA_INOPERANTE) {
                api::CFormBuilderMT campos;
                campos.Add<api::CTextFieldMT>(api::SPoint{20, 1},
                    std::make_shared<api::CFixedText>(api::ETextAlignment(2), "URNA ELETRÔNICA INOPERANTE"));
                campos.Add<api::CTextFieldMT>(api::SPoint{20, 2},
                    std::make_shared<api::CFixedText>(api::ETextAlignment(2), "Siga as instruções na tela do eleitor"));
                campos.CriaForm("")->Show();                        // func 1694, IForm slot 2
                m_bParar = true;
                break;
            }
            if (mensagem.id == MSG_TERMINA_OPERADOR) {
                m_bParar = true;
                break;
            }

            LogDebug("%s: 1 msg[%d] context[%s]", "Run", mensagem.id, NomeEstado(m_pContexto.get()));
            if (m_pContexto->AceitaMensagens()) {                   // func 3842: state +8
                m_pContexto->ProcessMessage(static_cast<uebyte>(mensagem.id));   // func 3843 (slot 6)
                m_pContexto->VerificaTrocaEstado();
            }
        }
        if (m_bParar)
            break;

        // ---- 2. one key --------------------------------------------------------------------------
        if (m_pContexto->AceitaTeclado() && teclado.HasKey()) {      // func 5909 (+9), IInputMT slot 3
            LogDebug("%s: 2 hasKey[%d] context[%s]", "Run", teclado.HasKey(), NomeEstado(m_pContexto.get()));
            m_pContexto->ProcessInput();                             // func 5911 (slot 7)
            processou = true;
            m_pContexto->VerificaTrocaEstado();
        }

        // ---- 3. timers ---------------------------------------------------------------------------
        const std::vector<uebyte> expirados = GetTicksExpirados();  // func 5450 (gettimeofday based)
        if (!expirados.empty() && m_pContexto->AceitaTicks()) {     // func 5908 (+10)
            for (const uebyte tick : expirados)
                m_pContexto->ProcessTick(tick);                      // func 5910 (slot 8)
            processou = true;
        }
        m_pContexto->VerificaTrocaEstado();

        m_pImpl->Yield();                                            // IThreadImpl slot 5
        if (!processou && m_estado == 1) {
            m_pSync->Lock();   m_bDormindo = true;   m_pSync->Unlock();      // ISyncCtl slots 2/3
            usleep(50000);
            m_pSync->Lock();   m_bDormindo = false;  m_pSync->Unlock();
        }
    }
}

}  // namespace vota
