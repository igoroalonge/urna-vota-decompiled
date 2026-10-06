// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cbuzzfieldmt.cpp (path inferred).
#include "api/gui/cbuzzfieldmt.h"

#include "api/gui/iscreen.h"

namespace api {

// wasm func 11049 (slot 2)
// IScreenMT slot 6. In the web build simulador::CWasmScreenMT::vf6 (func 8787) ignores both arguments and
// only refreshes the LCD: the simulator's microterminal has no buzzer.
void CBuzzFieldMT::Draw(IScreenMT& tela) const
{
    tela.Buzz(m_param1, m_param2);                // IScreenMT slot 6      name inferred
}

} // namespace api
