// FRAGMENT reconstructed by unit u39 from vota_web_wasm.wasm.
// Original file (path inferred): uenux2/src/app/vota/operador/outrasopcoes/chabilitaaudiomanualmente.cpp
// Class declaration: vota/operador/estadosoperador.u34.h (unit u34); constructor/GetInst inlined into
// CConfirmaAudio::ProcessInput (func 10740), reconstructed in cconfirmaaudio.u34.cpp.
//
// "Habilita áudio manualmente": option "Ativar áudio" / "Desativar áudio" of the mesário's "outras opções"
// menu, after CConfirmaAudio. Toggles the flag that makes the next released voter vote with audio
// (IInformacaoThreadOperador slot 6/7), shows "ÁUDIO ATIVADO" / "ÁUDIO DESATIVADO" for one second and
// returns to the identification screen.
//
// RTTI: comum::CAppState <- vota::CHabilitaAudioManualmente (typeinfo @1586996, vtable @1586960, 28 bytes:
//   +12 m_form, +20 std::shared_ptr<std::string> m_texto)   [2] StartState 10751, everything else default.
//
// WEB BUILD: dead code (operator thread not run). If it ever ran in the browser, CSystem::Sleep(1000) would
// ABORT the module (emscripten_sleep without ASYNCIFY), see the unit doc.
#include "api/util/csystem.h"
#include "vota/log/clogvota.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/operador/estadosoperador.u34.h"
#include "vota/operador/leidentidade/cpedeidentidade.h"

namespace vota {

// wasm func 10751 - vtable slot 2
void CHabilitaAudioManualmente::StartState()
{
    // funcs 1687 / 5415 are one-line out-of-line calls of IInformacaoThreadOperador slots 6 / 7 (shared with
    // other callers; cescolheopcao.cpp keeps a local copy of 1687 in its anonymous namespace).
    auto& info = impl::IInformacaoThreadOperador::GetInst();                   // func 599
    info.SetAudioHabilitadoManualmente(!info.GetAudioHabilitadoManualmente()); // api_f5415(api_f1687() ^ 1)

    if (info.GetAudioHabilitadoManualmente()) {                                // api_f1687 again
        *m_texto = "ÁUDIO ATIVADO";                                            // ecourna_f276 (assign), +20
        CLogVota::GetInst().Loga("Áudio ativado pelo mesário");               // api_f233
    } else {
        *m_texto = "ÁUDIO DESATIVADO";
        CLogVota::GetInst().Loga("Áudio desativado pelo mesário");
    }
    m_form->Show();                                                            // +12

    // Compiled as `if (byte @1584624 == 1) emscripten_sleep(1000)` (the byte is 1 and never written).
    api::CSystem::Sleep(1000);

    m_proximoEstado = &CPedeIdentidade::GetInst();                             // func 652
}

}  // namespace vota
