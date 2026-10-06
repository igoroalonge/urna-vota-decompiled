// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred by u17): uenux2/src/app/vota/operador/leidentidade/cidentidadeinvalida.cpp
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/leidentidade/cidentidadeinvalida.h"

#include "vota/log/clogvota.h"

namespace vota {

// wasm func 10669 - vtable slot 2
void CIdentidadeInvalida::StartState()
{
    CLogVota::GetInst().Loga(api::ESeveridade{2}, "Identificador do eleitor digitado inválido");   // CLoga::loga inline
    m_form->Show();
    m_proximoEstado = this;
}

}  // namespace vota
