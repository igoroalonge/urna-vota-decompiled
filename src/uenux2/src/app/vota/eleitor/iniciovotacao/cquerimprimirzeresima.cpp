// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred by u07): uenux2/src/app/vota/eleitor/iniciovotacao/cquerimprimirzeresima.cpp
// GetInst (5957): cquerimprimirzeresima.u07.cpp. ProcessInput (11920): src/uenux2/src/app/vota/u20-foreign-fragments.cpp.
#include "vota/eleitor/iniciovotacao/cquerimprimirzeresima.h"

#include "vota/log/clogvota.h"

namespace vota {

// wasm func 11921 - vtable slot 2
void CQuerImprimirZeresima::StartState()
{
    CLogVota::GetInst().LogaMesarioIndagadoImprimirZeresima();   // func 5882 (clogvota.u09.cpp)
    m_tela->Show();
    m_proximoEstado = this;
}

}  // namespace vota
