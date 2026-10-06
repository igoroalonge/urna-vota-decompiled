// Reconstructed from vota_web_wasm.wasm (unit u27). Original: uenux2/src/app/vota/operador/outrasopcoes/caguardaeleitoresvotarem.cpp
// (attested: srcloc caguardaeleitoresvotarem.cpp:47 in StartState).
//
// "Aguarda eleitores votarem": the mesário answered "CORRIGE: não" to "Todas as pessoas presentes já
// votaram?" (CPerguntaFilaEleitorVazia) while trying to close the vote. The MT shows for 3 seconds
//     Aguarde até todos os
//     eleitores presentes votarem
// (constructor in another unit) and returns to the identification screen.
//
// RTTI: comum::CAppState <- vota::CAguardaEleitoresVotarem (vtable @1587628; slot 2 StartState = func 10718).
// 20 bytes, CAppState(0) (no keys, messages or ticks), form at +12; GetInst + constructor are inlined into
// CPerguntaFilaEleitorVazia::ProcessInput (func 10713, lazy singleton @1905168).
//
// WEB BUILD: dead code. If it ever ran in the browser it would ABORT: api::CSystem::Sleep(3000) is
// compiled as `if (byte @1584624 == 1) emscripten_sleep(3000)`, the byte is 1 and never written, and
// the glue's _emscripten_sleep aborts (no ASYNCIFY). The operator harness stubs emscripten_sleep.
#include "api/hwil/iinput.h"
#include "api/util/csystem.h"
#include "comum/cappstate.h"
#include "vota/operador/leidentidade/cpedeidentidade.h"
#include "vota/operador/outrasopcoes/caguardaeleitoresvotarem.h"

namespace vota {

// wasm func 10718 (vtable slot 2, srcloc :47)
void CAguardaEleitoresVotarem::StartState()
{
    m_form->Show();                                                             // IForm slot 2
    api::CSystem::Sleep(3000);                                                  // emscripten_sleep(3000), see above
    api::CPolySingletonList::instance<api::IInputMT>().Flush();                 // :47, IInput slot 4: drop keys typed meanwhile
    m_proximoEstado = &CPedeIdentidade::GetInst();                              // func 652
}

}  // namespace vota
