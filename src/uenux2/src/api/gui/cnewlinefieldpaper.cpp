// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cnewlinefieldpaper.cpp (path inferred).
#include "api/gui/cnewlinefieldpaper.h"

#include "api/gui/ipaper.h"

namespace api {

// wasm func 11012 (slot 2)
void CNewLineFieldPaper::Draw(IPaper& papel) const
{
    for (int i = 0; i < m_linhas; ++i)          // unsigned compare in the binary (c < a[6])
        papel.NewLine();                         // IPaper slot 4      name inferred
}

} // namespace api
