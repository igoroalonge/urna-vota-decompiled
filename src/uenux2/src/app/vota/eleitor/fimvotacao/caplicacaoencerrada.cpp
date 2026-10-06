// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred by u09): uenux2/src/app/vota/eleitor/fimvotacao/caplicacaoencerrada.cpp
// Constructor + GetInst (3877): caplicacaoencerrada.u09.cpp.
//
// WEB BUILD: unreachable (the closing never starts in the page, docs/10-boletim-de-urna.md §5.1).
#include "vota/eleitor/fimvotacao/caplicacaoencerrada.h"

#include "vota/log/clogvota.h"

namespace vota {

// wasm func 12058 - vtable slot 2
void CAplicacaoEncerrada::StartState()
{
    m_proximoEstado = this;
    CLogVota::GetInst().Loga("Votação encerrada");        // api_f233 (severity 1)
    m_tela->Show();                                       // +28
}

}  // namespace vota
