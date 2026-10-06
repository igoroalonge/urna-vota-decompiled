// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cbeepfieldmt.cpp (path inferred).
#include "api/gui/cbeepfieldmt.h"

#include "api/gui/iscreen.h"

namespace api {

// wasm func 11053 (slot 2)
// IScreenMT slot 7 (web build: simulador::CWasmScreenMT::vf7, func 5039, only refreshes the LCD).
void CBeepFieldMT::Draw(IScreenMT& tela) const
{
    tela.Beep(m_quantidade);                      // IScreenMT slot 7      name inferred
}

} // namespace api
