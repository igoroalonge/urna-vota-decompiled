// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaozeresimatardia.cpp
// ProcessInput (11931): src/uenux2/src/app/vota/u20-foreign-fragments.cpp.
#include "vota/eleitor/iniciovotacao/cimpressaozeresimatardia.h"

#include "vota/log/clogvota.h"

namespace vota {

// wasm func 11932 - vtable slot 2
void CImpressaoZeresimaTardia::StartState()
{
    CLogVota::GetInst().Loga("Mesário indagado se horário da urna está correto");   // api_f233 (severity 1)
    m_tela->Show();
    m_proximoEstado = this;
}

}  // namespace vota
