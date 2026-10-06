// FRAGMENT reconstructed by unit u07 from vota_web_wasm.wasm.
// Original file (path inferred): uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.cpp
// vota::CProgressoEncerramento : comum::CAbstractTelaProgresso (typeinfo @1540616, vtable @1540596).
// Progress screen of the "encerramento" (closing of the vote: BU generation, printing, recording).
// layout: +0 vptr, +4 shared_ptr<form> m_tela, +12 shared_ptr<api::CProgressBar> m_barra, +20 std::mutex m_mutex
// vtable: [0] ~ (3907) [1] deleting (12104) [2] Inicia (12103: bar = min, show)  [3] Finaliza (12102: bar = max)
//         [4] Avanca (12101)
#include "vota/eleitor/fimvotacao/cprogressoencerramento.h"

#include <mutex>

namespace vota {

// wasm func 12101 (vtable slot 4)                                                    // name inferred
void CProgressoEncerramento::Avanca()
{
    std::lock_guard<std::mutex> lock(m_mutex);                                   // +20 (only the unlock stub survives)
    m_barra->Incrementa();                                                       // func 5508
}

} // namespace vota
