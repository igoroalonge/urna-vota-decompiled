// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cqrcodeimagefieldpaper.cpp (path inferred).
#include "api/gui/cqrcodeimagefieldpaper.h"

#include "api/gui/ipaper.h"

namespace api {

// wasm func 10962 (slot 0) / 10961 (slot 1 = the same + operator delete): frees m_bitmap, then the base
// IFormFieldBase<IPaper> (m_nome).
CQRCodeImageFieldPaper::~CQRCodeImageFieldPaper() = default;

// wasm func 10963 (slot 2)
// Hands the raster to the printer driver (IPaper slot 12). On the urna the driver serialises it as the image
// block `0B | u32 len | u32 largura | u8 escala | bitmap` (docs/bu/qrcode.md); in the web build the paper is
// simulador::CWasmNullPaper and slot 12 is a no-op (ICF 1870), so nothing is printed.
void CQRCodeImageFieldPaper::Draw(IPaper& papel) const
{
    papel.PrintImage(m_bitmap, m_largura, m_escala);                  // IPaper slot 12      name inferred
}

} // namespace api
