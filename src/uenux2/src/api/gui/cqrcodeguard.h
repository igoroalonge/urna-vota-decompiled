// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/cqrcodeguard.h (path
// inferred from cqrcodeguard.cpp, attested by the srcloc record :23 of the constructor).
//
// api::CQRCodeGuard - RAII owner of a libqrencode `QRcode*` (no RTTI, no vtable, 4 bytes). Built on the
// stack by the two QR image builders:
//   CQRCodeImage::GetImage     (screen, wasm func 10967)   -> 8-bit grey BMP
//   CQRCodeImagePaper::MontaImagem (printer, wasm func 2772) -> 1-bit raster for the thermal printer
// The destructor (QRcode_free, wasm func 1254) has no out-of-line copy: it is inlined into both callers.
#pragma once

#include <string>

extern "C" {
#include <qrencode.h>   // libqrencode 4.1.1 (see docs/libraries/boost-fmt-and-small-libs.md §4)
}

namespace api {

class CQRCodeGuard {
public:
    explicit CQRCodeGuard(const std::string& texto);   // wasm func 5502 (srcloc :23)
    ~CQRCodeGuard() { QRcode_free(m_qrcode); }         // inlined in the callers

    CQRCodeGuard(const CQRCodeGuard&) = delete;
    CQRCodeGuard& operator=(const CQRCodeGuard&) = delete;

    const QRcode* operator->() const { return m_qrcode; }   // ->width, ->data[i] & 1 = dark module

private:
    QRcode* m_qrcode = nullptr;   // +0
};

} // namespace api
