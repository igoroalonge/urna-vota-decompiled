// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original files: uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.cpp (path inferred) and
//                 uenux2/src/app/vota/operador/outrasopcoes/caguardaeleitoresvotarem.cpp (attested; owner u27)
// Class declarations: ../estadosoperador.u34.h. CPerguntaFilaEleitorVazia's constructor/GetInst (5426) are in
// u27-foreign-fragments.cpp; its StartState (10715) is in unit u39.
//
// Closing the vote ("encerramento"): after "2-Encerrar votação" in the MT menu, CIniciaFinalizacao asks
// "Todas as pessoas presentes já votaram?" (voter-training mode, or after the closing time while somebody is
// still in the queue). CONFIRMA = yes -> the presidente types his título (CPedeTituloEncerramento); CORRIGE =
// no -> "Aguarde até todos os eleitores presentes votarem" for 3 s, then the identification screen.
//
// WEB BUILD: dead code (operator thread not run). CAguardaEleitoresVotarem::StartState (u27) would abort in
// the browser: api::CSystem::Sleep(3000) calls emscripten_sleep, which needs Asyncify.
#include <format>
#include <memory>
#include <mutex>
#include <string>

#include "api/gui/cformbuildermt.h"
#include "api/uelog/cloga.h"
#include "vota/log/clogvota.h"
#include "vota/operador/estadosoperador.u34.h"
#include "vota/operador/outrasopcoes/cpedetituloencerramento.h"

namespace vota {

namespace {
constexpr auto CENTRO = api::ETextAlignment(2);
}

// Constructor - inlined into CPerguntaFilaEleitorVazia::ProcessInput (wasm func 10713).
// 20 bytes: CAppState(0) (accepts nothing: it only waits in its StartState), m_form = {} (+12/+16).
CAguardaEleitoresVotarem::CAguardaEleitoresVotarem()
    : comum::CAppState(0)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CTextFieldMT>(api::SPoint{1, 2},
                                  std::make_shared<api::CFixedText>(CENTRO, "Aguarde até todos os"));          // func 180
    campos.Add<api::CTextFieldMT>(api::SPoint{1, 3},
                                  std::make_shared<api::CFixedText>(CENTRO, "eleitores presentes votarem"));   // func 180
    m_form = campos.CriaFormInterativo("", true);                                                              // func 301
}

// GetInst - inlined into wasm func 10713: static unique_ptr @1905168, mutex @1905144.
CAguardaEleitoresVotarem& CAguardaEleitoresVotarem::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CAguardaEleitoresVotarem> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CAguardaEleitoresVotarem());
    return *s_inst;
}

// wasm func 10713 - vtable slot 7 (ProcessInput). srcloc cinteractiveform.h:57 @1587748.
void CPerguntaFilaEleitorVazia::ProcessInput()
{
    bool todosVotaram;
    switch (m_form->Read()) {
    case api::EInputResult::CONFIRMA:                                       // 9: "sim"
        m_proximoEstado = &CPedeTituloEncerramento::GetInst();              // func 3631
        todosVotaram = true;
        break;
    case api::EInputResult::CORRIGE:                                        // 5: "não"
        m_proximoEstado = &CAguardaEleitoresVotarem::GetInst();
        todosVotaram = false;
        break;
    default:
        return;
    }
    // IEventosLog::Loga inlined: api::CLoga::loga(m_aplicativo, severity 1, text)
    CLogVota::GetInst().Loga(std::format("Todas as pessoas presentes já votaram? {}",
                                         todosVotaram ? "SIM" : "NÃO"));   // @3236, "SIM" @327376, "NÃO" @326530
}

}  // namespace vota
