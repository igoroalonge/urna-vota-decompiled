// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cclockfieldmt.cpp (path inferred).
#include "api/gui/cclockfieldmt.h"

#include "api/gui/iscreen.h"

namespace api {

// wasm func 11048 (slot 2)
// IScreenMT slot 12: in the web build simulador::CWasmScreenMT::vf12 (func 8757) formats the current time and
// writes it at `m_pos` through slot 3.
void CClockFieldMT::Draw(IScreenMT& tela) const
{
    tela.ShowClock(m_pos);                        // IScreenMT slot 12      name inferred
}

} // namespace api
