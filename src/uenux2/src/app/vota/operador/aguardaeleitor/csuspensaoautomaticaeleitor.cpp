// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original: uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp
// (attested by the std::source_location record of the lambda in DisparaSinalizacaoSonora, line 89).
//
// Web build: never executed (operator thread not run). If it were, DisparaSinalizacaoSonora would throw
// std::system_error(ENOTSUP = 138, "thread constructor failed") on the first call - the build has no
// pthreads, and the optimizer folded std::thread's constructor into an unconditional throw (func 5392).
#include "vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.h"

#include <map>
#include <string>
#include <thread>

#include "api/gui/iscreen.h"                                    // api::IScreenMT
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/eleitor/celeitorvotando.h"                       // EMensagemEleitor
#include "vota/log/clogvota.h"
#include "vota/operador/cthreadoperador.h"
#include "vota/operador/aguardaeleitor/cmostraeleitorvotando.h"
#include "vota/operador/comum/csincronismooperador.h"
#include "vota/operador/leidentidade/cpedeidentidade.h"

namespace vota {

// wasm func 2732 (slot 0) / 10411 (slot 1, deleting): releases the three shared_ptrs.
CSuspensaoAutomaticaEleitor::~CSuspensaoAutomaticaEleitor() = default;

// wasm func 10414 (table slot 4243, registered with atexit)                     name inferred
// Exit-time destruction of the singleton: `s_inst.reset()` of the static unique_ptr @1909400.

// ---------------------------------------------------------------------------------------------
// wasm func 3612                                                               name inferred
void CSuspensaoAutomaticaEleitor::AtualizaTextoContagem()
{
    const auto n = m_segundosRestantes.count();
    *m_textoContagem = "Suspensão automática em " + std::to_string(n) +
                       (n == 1 ? " segundo..." : " segundos...");
}

// ---------------------------------------------------------------------------------------------
// wasm func 5392 (std::thread constructor + detach, inlined) and func 10410 (the lambda, line 89,
// run through std::__thread_proxy: sets the thread-local __thread_struct, calls the lambda, frees it).
// The beep is played on another thread so that the 1 s tick is not delayed.
void CSuspensaoAutomaticaEleitor::DisparaSinalizacaoSonora()
{
    std::thread([] {
        api::CPolySingletonList::instance<api::IScreenMT>().Bipa(50, 3);   // line 89; IScreenMT slot 6 (50, 3) ?
    }).detach();
    // In this build: new __thread_struct, then __throw_system_error(138 /* ENOTSUP */, "thread constructor
    // failed") unconditionally - pthread_create is a stub that always fails without -pthread.
}

// ---------------------------------------------------------------------------------------------
// wasm func 10409 - vtable slot 2                                             name from slot order
void CSuspensaoAutomaticaEleitor::StartState()
{
    CThreadOperador& operador = CThreadOperador::GetInst();
    // Make sure the 1 s tick exists and is stopped (the singleton survives between voters). The tick
    // table of the thread (CThreadVota::m_ticks, +20) is copied into a local map id -> "running";
    // funcs 3613 (tree destroy) belongs to that temporary map.
    const std::map<uebyte, bool> ticks = operador.GetEstadoTicks();       // inlined, name inferred
    if (const auto it = ticks.find(m_tick); it == ticks.end())
        m_tick = operador.CriaTick(1000);                                   // func 807
    else if (it->second)
        operador.StopTick(m_tick);                                          // func 422

    *m_textoSituacao = m_votouParcialmente ? "Votou parcialmente" : "Não votou";
    m_segundosRestantes = std::chrono::seconds{10};
    m_suspensaoEnviada = false;
    AtualizaTextoContagem();
    DisparaSinalizacaoSonora();
    operador.StartTick(m_tick);
    m_form->Show();
    m_proximoEstado = this;
}

// ---------------------------------------------------------------------------------------------
// wasm func 10408 - vtable slot 5                                             name from slot order
void CSuspensaoAutomaticaEleitor::FinishState()
{
    CThreadOperador::GetInst().StopTick(m_tick);
    m_suspensaoEnviada = false;
}

// ---------------------------------------------------------------------------------------------
// wasm func 10404 - vtable slot 6                                             name from slot order
void CSuspensaoAutomaticaEleitor::ProcessMessage(uebyte mensagem)
{
    const auto msg = static_cast<EMensagemOperadorRecebida>(mensagem);
    if (msg == EMensagemOperadorRecebida::EleitorVoltouADigitar) {          // 5: the voter reacted in time
        CThreadOperador::GetInst().StopTick(m_tick);
        m_proximoEstado = &CMostraEleitorVotando::GetInst();                // func 1150
        return;
    }
    if (!m_suspensaoEnviada)
        return;
    switch (msg) {
    case EMensagemOperadorRecebida::EleitorNaoVotou:                        // 2: suspended, nothing confirmed
        m_suspensaoEnviada = false;
        m_proximoEstado = &CPedeIdentidade::GetInst();                      // func 652 (next voter)
        break;
    case EMensagemOperadorRecebida::SincronizaVoto: {                       // 13: partial vote recorded
        m_suspensaoEnviada = false;
        auto& sincronismo = CSincronismoOperador::GetInst();                // func 1897
        sincronismo.m_suspensaoAutomatica = true;
        m_proximoEstado = &sincronismo;
        break;
    }
    default:
        break;
    }
}

// ---------------------------------------------------------------------------------------------
// wasm func 10407 - vtable slot 7                                             name from slot order
void CSuspensaoAutomaticaEleitor::ProcessInput()
{
    if (m_form->Read() != api::EInputResult::CORRIGE)                       // cinteractiveform.h:57 inlined
        return;
    if (m_suspensaoEnviada)
        return;
    CThreadOperador::GetInst().StopTick(m_tick);
    CLogVota::GetInst().Loga("Mesário abortou processo de suspensão");       // func 4535 -> api_f233 (level 1)
    CThreadEleitor::GetInst().EnviaMensagem({EMensagemEleitor::ContinuaVotacao}, 1);   // 4
    m_proximoEstado = &CMostraEleitorVotando::GetInst();
}

// ---------------------------------------------------------------------------------------------
// wasm func 10405 - vtable slot 8 (the tools' curated name ProcessTick)
void CSuspensaoAutomaticaEleitor::ProcessTick(uebyte tick)
{
    if (tick != m_tick || m_suspensaoEnviada)
        return;
    if (m_segundosRestantes >= std::chrono::seconds{2}) {
        --m_segundosRestantes;
        AtualizaTextoContagem();
        DisparaSinalizacaoSonora();
        m_form->Show();
        return;
    }
    m_suspensaoEnviada = true;
    m_segundosRestantes = std::chrono::seconds{0};
    CThreadOperador::GetInst().StopTick(m_tick);
    AtualizaTextoContagem();                                                // "... em 0 segundos..."
    m_form->Show();
    CThreadEleitor::GetInst().EnviaMensagem({EMensagemEleitor::SuspensaoAutomatica}, 1);  // 3
}

// wasm func 4535 (tools: "api_f4535")                                          name inferred
// CLogVota helper: builds the 37-byte literal "Mesário abortou processo de suspensão" and calls
// CLogVota::Loga (api_f233 = CLoga::loga(level 1)). Also used by CPerguntaCodigoSuspensao::ProcessInput.
//
// wasm func 3613 (tools: "vota_f3613"): std::__tree<std::__value_type<uebyte, bool>>::destroy - the
// recursive node deleter of the temporary map in StartState (library instantiation, not reconstructed).

}  // namespace vota
