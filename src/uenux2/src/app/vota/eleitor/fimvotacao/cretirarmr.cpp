// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cretirarmr.cpp
//
// srcloc evidence:
//   :54  StartState         Assert (vota.GetEstadoVota() == EAVENCERRADA)          (3476)
//   :55  StartState         Assert (vota.GetEstadoEncerramento() == EAERETIRAMR)   (3477)
//   :56  StartState         comum::IInterfaceInit lookup (GetDemoMode)
//   :78  ProcessInput       comum::IInterfaceInit lookup (GetDemoMode)
//   :96  AguardaRetiradaMR  comum::IInterfaceInit lookup
//
// Web build: simulador::CWasmInit answers message 18 (IsMRPresenteSemHabilitar) with 1, which the
// caller turns into "not present", so the wait loop below would not spin; the state is not reached anyway.

#include "vota/eleitor/fimvotacao/cretirarmr.h"

#include "api/util/cwait.h"
#include "comum/cappinfo.h"
#include "comum/iinterfaceinit.h"
#include "vota/eleitor/fimvotacao/cemitirmaisbu.h"
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;
using comum::md::estadoaplicacao::EEstadoEncerramento;

// wasm func 12031 — vtable slot 2
void CRetirarMR::StartState()
{
    auto& vota = comum::CAppInfo::GetInst().GetVota();
    vota.SetEstadoEncerramento(EEstadoEncerramento::EAERETIRAMR);                  // 51
    comum::SalvaEstado();                                                          // func 491
    UE_ASSERT(vota.GetEstadoVota() == EEstadoVota::EAVENCERRADA);                  // :54
    UE_ASSERT(vota.GetEstadoEncerramento() == EEstadoEncerramento::EAERETIRAMR);   // :55 (always true here)

    if (comum::IInterfaceInit::GetInst().GetDemoMode()) {                         // :56 (func 729)
        vota.SetEstadoEncerramento(EEstadoEncerramento::EAEFIMDOSTRABALHOS);       // 52 (not saved)
        m_telaModoDemo->Exibe();
    } else {
        m_telaRetireMR->Exibe();
        CLogVota::GetInst().Loga("Solicitada a retirada da mídia de resultado");  // CLoga::loga level 1
        AguardaRetiradaMR();
        CLogVota::GetInst().Loga("Mídia de resultado retirada");
        vota.SetEstadoEncerramento(EEstadoEncerramento::EAEFIMDOSTRABALHOS);       // 52
        comum::SalvaEstado();
        m_telaFimTrabalhos->Exibe();
    }
    m_proximoEstado = this;
}

// srcloc :96 — inlined into func 12031. Busy-waits (250 ms steps) while the MR is present.
bool CRetirarMR::AguardaRetiradaMR()
{
    auto& init = comum::IInterfaceInit::GetInst();                                 // :96
    while (init.IsMRPresenteSemHabilitar())                                        // func 2862 (message 18)
        api::CWait::Sleep(250);   // emscripten_sleep(250) when the byte @1584624 == 1: would abort this
                                  // build (no Asyncify), but CWasmInit always reports the MR absent, so
                                  // the loop body is never entered in the simulator
    return true;
}

// wasm func 12030 — vtable slot 7
void CRetirarMR::ProcessInput()
{
    const bool demo = comum::IInterfaceInit::GetInst().GetDemoMode();              // :78
    const auto& tela = demo ? m_telaModoDemo : m_telaFimTrabalhos;                 // +28 / +20
    if (tela->Read() == api::EInputResult::Confirma)                               // cinteractiveform.h:57
        m_proximoEstado = &CEmitirMaisBU::GetInst();                               // func 3879
}

}  // namespace vota
