// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred by u10): uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp
// Constructor/GetInst (2735): unit u10. ProcessInput (10443): operador/u17-foreign-fragments.cpp.
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/confirmaidentidade/cverificadadoeleitor.h"

#include "vota/log/clogvota.h"

namespace vota {

// wasm func 10444 - vtable slot 2
void CVerificaDadoEleitor::StartState()
{
    CLogVota::GetInst().Loga("Solicitação de dado pessoal do eleitor para habilitação manual");   // api_f233
    m_form->Show();
    m_proximoEstado = this;
}

}  // namespace vota
