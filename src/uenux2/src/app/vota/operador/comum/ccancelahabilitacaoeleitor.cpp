// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/operador/comum/ccancelahabilitacaoeleitor.cpp
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/comum/ccancelahabilitacaoeleitor.h"

#include "vota/log/clogvota.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/operador/leidentidade/cpedeidentidade.h"

namespace vota {

// wasm func 10646 - vtable slot 2
void CCancelaHabilitacaoEleitor::StartState()
{
    // Audio switched on by hand for this voter ("outras opções" -> CHabilitaAudioManualmente) is dropped
    // together with the habilitação.
    // func 1687 = one-line out-of-line call of IInformacaoThreadOperador slot 6, shared by six callers.
    if (impl::IInformacaoThreadOperador::GetInst().GetAudioHabilitadoManualmente())
        CLogVota::GetInst().LogaAudioDesativadoFimVotacao();       // vota_f4529: "Áudio desativado pelo fim da votação"
                                                                   //   (name inferred; severity 1)
    impl::IInformacaoThreadOperador::GetInst().LimpaDadosHabilitacao();   // func 599, slot 2
    m_proximoEstado = &CPedeIdentidade::GetInst();                 // func 652
}

}  // namespace vota
