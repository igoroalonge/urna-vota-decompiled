// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cpreshowclearmt.cpp (path inferred).
#include "api/gui/cpreshowclearmt.h"

#include "api/gui/iscreen.h"

namespace api {

// wasm func 11047 (slot 2)
void CPreShowClearMT::PreShow(IScreenMT& tela)
{
    tela.Clear();                                 // IScreenMT slot 2 (CWasmScreenMT: 4 lines of 40 spaces)
    tela.ResetIndicators();                       // IScreenMT slot 13 (no-op in CWasmScreenMT)  name unknown ?
}

} // namespace api
