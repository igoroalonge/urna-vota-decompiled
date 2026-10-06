// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cpreshowclearscreen.cpp (path inferred).
#include "api/gui/cpreshowclearscreen.h"

#include "api/gui/iscreen.h"

namespace api {

// wasm func 11139 (slot 2)
void CPreShowClearScreen::PreShow(IScreen& tela)
{
    tela.Clear(1);                                // IScreen slot 4, colour 1 = white background
}

} // namespace api
