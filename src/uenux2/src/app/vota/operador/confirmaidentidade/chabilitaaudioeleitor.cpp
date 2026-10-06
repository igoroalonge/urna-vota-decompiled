// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred by u33): uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.cpp
// Constructor + GetInst (2743): chabilitaaudioeleitor.u33.cpp. ProcessInput (10523): operador/u19-foreign-fragments.cpp.
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/confirmaidentidade/chabilitaaudioeleitor.h"

#include "vota/log/clogvota.h"

namespace vota {

// wasm func 10524 - vtable slot 2
void CHabilitaAudioEleitor::StartState()
{
    CLogVota::GetInst().Loga("Solicitado ao mesário que conecte o fone de ouvido");   // api_f233 (severity 1)
    m_form->Show();
    m_proximoEstado = this;
}

}  // namespace vota
