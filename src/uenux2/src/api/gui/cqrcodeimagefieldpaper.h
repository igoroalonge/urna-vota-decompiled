// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cqrcodeimagefieldpaper.h (path inferred from cqrcodeimagepaper.cpp, whose
// srclocs attest the directory).
//
// api::CQRCodeImageFieldPaper : api::IFormField<api::IPaper>   (typeinfo 1582112, vtable @1582080, 44 bytes)
// A QR code printed on paper. This is how the QR codes of the BOLETIM DE URNA reach the printer: vota::CGeraBU
// (func 12110) and the "BU das outras obrigatórias" path (func 5591) first compute the raster themselves with
// CQRCodeImagePaper::MontaImagem (func 2772; unit u16: 1 bit per module, 2-module quiet zone, scale =
// min(400 / largura, 4) dots per module), then call CPaperFormBuilder::AddQRCode (func 2775), which only
// copies that QrcodeData into this field and appends it.
//   +24 std::vector<uebyte> m_bitmap   +36 size_t m_largura (modules per side)   +40 uebyte m_escala
// Vtable: 0 dtor (10962)  1 deleting (10961)  2 Draw (10963)  3/4 no-op  5 RedrawIfDirty  6 SetForm
//         7 GetClassName (ICF 11014: "IFormField<IPaper>")
#pragma once

#include <cstddef>
#include <vector>

#include "api/gui/cqrcodeimagepaper.h"   // api::CQRCodeImagePaper::QrcodeData (unit u16)
#include "api/gui/iformfield.h"

namespace api {

class CQRCodeImageFieldPaper : public IFormField<IPaper> {
public:
    // Inlined in CPaperFormBuilder::AddQRCode (func 2775): copies the three members of QrcodeData.
    explicit CQRCodeImageFieldPaper(const CQRCodeImagePaper::QrcodeData& qrcode)
        : m_bitmap(qrcode.bitmap), m_largura(qrcode.largura), m_escala(qrcode.escala)
    {
    }
    ~CQRCodeImageFieldPaper() override;                              // wasm 10962 / 10961

    void Draw(IPaper& papel) const override;                         // wasm 10963

private:
    std::vector<uebyte> m_bitmap;    // +24
    std::size_t         m_largura;   // +36
    uebyte              m_escala;    // +40
};

} // namespace api
