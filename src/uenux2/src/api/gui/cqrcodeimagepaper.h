// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/cqrcodeimagepaper.h (path
// inferred from cqrcodeimagepaper.cpp, attested by the srcloc records :35/:38/:49 of MontaImagem).
//
// api::CQRCodeImagePaper - builds the raster of a QR code for the thermal printer (the QR codes printed on
// the Boletim de Urna, "BU DIGITAL", and the "CERTIFICADO DIGITAL" ones). No RTTI: a namespace-like class
// with a static function. The result is handed to api::CQRCodeImageFieldPaper (unit u32) through
// CPaperFormBuilder::AddQRCode (func 2775, unit u08); the printer driver serialises it as the image
// block `0B | u32 len | u32 largura | u8 escala | bitmap` seen in real *-imgbu.dat files
// (docs/bu/qrcode.md §2).
#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace api {

class CQRCodeImagePaper {
public:
    struct QrcodeData {                 // attested name (srcloc return type), 20 bytes
        std::vector<uebyte> bitmap;     // +0   ceil(largura^2 / 8) bytes, 1 bit per module, LSB first,
                                        //      row-major, 1 = dark; includes a 2-module white border
        std::size_t largura = 0;        // +12  modules per side, border included (QR width + 4)
        uebyte escala = 0;              // +16  printer dots per module = min(400 / largura, 4)
    };

    // wasm func 2772 (srcloc :35, :38, :49). Every caller passes larguraMaxima = 400 (constant-folded).
    static QrcodeData MontaImagem(std::size_t larguraMaxima, const std::string& texto);
};

} // namespace api
