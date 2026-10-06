// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/cqrcodeimage.h (path
// inferred from cqrcodeimage.cpp, attested by the srcloc record :36 of MontaImagem).
//
// api::CQRCodeImage : api::IImage   (typeinfo 1582060, vtable @1582032, 32 bytes)
//
// An image source (IImage) whose picture is a QR code computed on demand from a text source. Used to show
// QR codes on the urna screen:
//   vota::CMostraQRCodeBU::StartState (func 12055, screen "telaQRCodeBU"): CQRCodeImage(380, <BU payload>)
//   vota::CMostraQRCodeCertificado::vf2 (func 12040): the "CERTIFICADO DIGITAL" QR code, also 380 px
//   func 3059 (the tools call it CImageFieldUpdate::CImageFieldUpdate because it inlines that constructor;
//   it is really a screen builder: status header + "RESUMO DA CORRESPONDÊNCIA: ..." text): the urna-state
//   QR code, CQRCodeImage(148, comum::CQRCodeDS{...}) inside a CImageFieldUpdate refreshed every 15000 ms.
//
// Vtable: 0 ~CQRCodeImage (10966 -> shared body 6060)  1 deleting dtor (10964 -> 6059)  2 GetImage (10967)
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "api/gui/iresource.h"   // api::IImage (slot 2: std::vector<uebyte> GetImage() const)   // name inferred

namespace api {

class CQRCodeImage : public IImage {
public:
    using TFonteTexto = std::function<std::string()>;

    CQRCodeImage(TPosition tamanho, const TFonteTexto& fonte);      // wasm func 3662   // param names inferred
    ~CQRCodeImage() override;                                       // wasm func 10966 / 10964

    // IImage slot 2: a complete .bmp file (8-bit grey, `tamanho` x `tamanho` pixels).
    std::vector<uebyte> GetImage() const override;                  // wasm func 10967   // name inferred

    // cqrcodeimage.cpp:36 - one byte per pixel (0 = black, 255 = white), rows bottom-up, 4-byte aligned.
    static std::vector<uebyte> MontaImagem(const TPosition tamanho, const std::string& texto);   // inlined

private:
    TPosition   m_tamanho;   // +4   side of the square image, in pixels
    TFonteTexto m_fonte;     // +8   payload generator (libc++ std::function: 16-byte buffer + __f_ at +24)
};

} // namespace api
