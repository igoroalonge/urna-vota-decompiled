// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred by u02): uenux2/src/app/vota/eleitor/cinspecionaurna.cpp
// ProcessInput (11797): src/uenux2/src/app/vota/eleitor/cestadosvota.u02.cpp.
#include "vota/eleitor/cinspecionaurna.h"

#include "vota/log/clogvota.h"

namespace vota {

// wasm func 11798 - vtable slot 2
void CInspecionaUrna::StartState()
{
    m_form->Show();
    m_proximoEstado = this;
    CLogVota::GetInst().Loga("Inspeção da urna iniciada");       // api_f233 (severity 1)
}

}  // namespace vota
