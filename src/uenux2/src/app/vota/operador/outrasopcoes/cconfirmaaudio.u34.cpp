// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original files: uenux2/src/app/vota/operador/outrasopcoes/cconfirmaaudio.cpp            (path inferred)
//                 uenux2/src/app/vota/operador/outrasopcoes/chabilitaaudiomanualmente.cpp (path inferred)
// Class declarations: ../estadosoperador.u34.h. CConfirmaAudio's constructor is inlined into
// CEscolheOpcao::ProcessInput (10689, cescolheopcao.cpp); its text source TextoConfirmaAudio is func 10742.
//
// "1-Ativar áudio" / "1-Desativar áudio" in the MT menu (only if the parametrisation allows the manual
// audio switch, ParametrosUrna.permitirHabManualAudio) asks "Deseja realmente ativar o áudio?" /
// "... desativar ...". CONFIRMA goes to CHabilitaAudioManualmente, whose StartState (10751, unit u39) flips
// the operator-side flag IInformacaoThreadOperador::AudioHabilitadoManualmente (slots 6/7; it posts NO
// message: the flag is read when the next voter is released), shows "ÁUDIO ATIVADO"/"ÁUDIO DESATIVADO",
// logs it, waits 1 s and returns to CPedeIdentidade; CORRIGE returns to the menu.
//
// WEB BUILD: dead code (operator thread not run). In the simulator the voter's audio is switched by the
// page (votaSetAudioEnabled / URL option), not by this menu.
#include <memory>
#include <mutex>
#include <source_location>
#include <string>

#include "api/gui/cformbuildermt.h"
#include "api/gui/ctextsource.h"
#include "vota/operador/estadosoperador.u34.h"
#include "vota/operador/outrasopcoes/cescolheopcao.h"

namespace vota {

namespace {
constexpr auto CENTRO = api::ETextAlignment(2);
}

// Constructor - inlined into CConfirmaAudio::ProcessInput (wasm func 10740). 28 bytes, CAppState(2).
CHabilitaAudioManualmente::CHabilitaAudioManualmente()
    : comum::CAppState(2)
    , m_texto(new std::string("áudio habilitado/desabilitado"))   // @138009 (29 chars); __shared_ptr_pointer
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);                                                   // func 435
    // api::CTextSource(const std::shared_ptr<std::string>&) (ctextsource.h:37) throws
    // CUeGuiError(4977, "Texto nulo") when the pointer is empty - impossible here.
    campos.Add<api::CTextFieldMT>(api::SPoint{1, 2},
                                  std::make_shared<api::CDataTextFmt<api::CTextSource>>(
                                      CENTRO, api::CTextSource(m_texto), "%s"));          // func 1152
    m_form = campos.CriaFormInterativo("", true);                                          // func 301
    // No control input: the state does not read keys (slot 7 is CAppState's no-op).
}

// GetInst - inlined into wasm func 10740: static unique_ptr @1904972, mutex @1904948.
CHabilitaAudioManualmente& CHabilitaAudioManualmente::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CHabilitaAudioManualmente> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CHabilitaAudioManualmente());
    return *s_inst;
}

// wasm func 10740 - vtable slot 7 (ProcessInput). srclocs cinteractiveform.h:57 @1587196 and
// ctextsource.h:37 @1587008 (from the inlined constructor above).
void CConfirmaAudio::ProcessInput()
{
    switch (m_form->Read()) {
    case api::EInputResult::CORRIGE:                                    // "CORRIGE: não"
        m_proximoEstado = &CEscolheOpcao::GetInst();                    // func 2753
        break;
    case api::EInputResult::CONFIRMA:                                   // "CONFIRMA: sim"
        m_proximoEstado = &CHabilitaAudioManualmente::GetInst();
        break;
    default:
        break;
    }
}

}  // namespace vota
