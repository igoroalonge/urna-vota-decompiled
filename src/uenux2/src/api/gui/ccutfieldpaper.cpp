// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/ccutfieldpaper.cpp (path inferred).
#include "api/gui/ccutfieldpaper.h"

#include "api/gui/ipaper.h"

namespace api {

// wasm func 11015 (slot 2)
void CCutFieldPaper::Draw(IPaper& papel) const
{
    papel.Cut();                                 // IPaper slot 3 (no-op in simulador::CWasmNullPaper)  name inferred
}

} // namespace api
