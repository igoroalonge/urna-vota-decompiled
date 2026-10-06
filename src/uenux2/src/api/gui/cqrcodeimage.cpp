// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/cqrcodeimage.cpp
// (srcloc record :36 in MontaImagem, which is inlined into the virtual GetImage, wasm func 10967).
#include "api/gui/cqrcodeimage.h"

#include <format>

#include "api/gui/cbmpconversor.h"   // api::CBmpConversor::CreateBitmapHeader (cbmpconversor.cpp:122, inlined)
#include "api/gui/cqrcodeguard.h"

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// wasm func 3662 (named by an earlier pass: stores the vtable and copy-constructs the std::function)
CQRCodeImage::CQRCodeImage(TPosition tamanho, const TFonteTexto& fonte)
    : m_tamanho(tamanho), m_fonte(fonte)
{
}

// wasm func 10966 (D1) and 10964 (D0). Both are 12-byte thunks into bodies shared (merge-similar, vtable
// passed as argument) with api::CDataText<std::function<std::string()>>, which has the same layout
// {vptr, 4-byte field, std::function at +8}:
//   wasm func 6060 (D1): vptr = vtable; destroy the std::function (__f_ == local buffer ? destroy() : destroy_deallocate())
//   wasm func 6059 (D0): the same + free(this)
// wasm funcs 12305 / 12303 are the CDataText<std::function<std::string()>> thunks into the same bodies.
CQRCodeImage::~CQRCodeImage() = default;

// cqrcodeimage.cpp:36 (static; inlined into GetImage)
std::vector<uebyte> CQRCodeImage::MontaImagem(const TPosition tamanho, const std::string& texto)
{
    const CQRCodeGuard qr(texto);                                                         // func 5502
    const int modulos = qr->width;
    if (tamanho < modulos + 4)                                                            // :36
        throw CUeGuiError(EUeGuiError{4948},
                       std::format("A largura do qrcode ({}) superou o espaço disponível ({})", modulos + 4,
                                   tamanho));

    const int escala = tamanho / (modulos + 4);                  // pixels per module (quiet zone: 2 modules)
    const int bytesPorLinha = tamanho + (tamanho % 4 != 0 ? 4 - tamanho % 4 : 0);   // BMP rows are 4-aligned
    const int tamanhoTotal = bytesPorLinha * tamanho;
    const int sobra = tamanho - (modulos + 4) * escala;          // unused pixels: all put on the left

    std::vector<uebyte> imagem;
    imagem.reserve(tamanhoTotal);
    std::vector<uebyte> linha(bytesPorLinha, 255);

    const std::size_t margem = 2 * escala * linha.size();       // 2 white modules
    imagem.insert(imagem.begin(), margem, 255);                  // bottom quiet zone (BMP is bottom-up)

    for (int y = modulos - 1; y >= 0; --y) {                     // last QR row first
        for (int x = 0; x < modulos; ++x) {
            const uebyte cor = (qr->data[y * modulos + x] & 1) ? 0 : 255;
            for (int p = 0; p < escala; ++p)
                linha.at(sobra + (x + 2) * escala + p) = cor;
        }
        for (int r = 0; r < escala; ++r)
            imagem.insert(imagem.end(), linha.begin(), linha.end());
    }
    imagem.insert(imagem.end(), margem, 255);                    // top quiet zone
    if (imagem.size() < static_cast<std::size_t>(tamanhoTotal))  // vertical remainder: inserted at the start,
        imagem.insert(imagem.begin(), tamanhoTotal - imagem.size(), 255);   // i.e. at the bottom of the picture
    return imagem;                                               // ~CQRCodeGuard -> QRcode_free (inlined)
}

// wasm func 10967 (vtable slot 2)                                                         // name inferred
std::vector<uebyte> CQRCodeImage::GetImage() const
{
    const std::vector<uebyte> pixels = MontaImagem(m_tamanho, m_fonte());   // std::bad_function_call if empty

    // CBmpConversor::CreateBitmapHeader(m_tamanho, m_tamanho, pixels.size(), 8) inlined (1078 bytes):
    //   "BM", file size = 1078 + n, data offset 1078; BITMAPINFOHEADER 40, width = height = m_tamanho
    //   (positive: bottom-up), 1 plane, 8 bpp, BI_RGB, image size n, 20000 x 20000 px/m, 0 colours used;
    //   palette of 256 grey entries {i, i, i, 0}.
    std::vector<uebyte> bmp = CBmpConversor::CreateBitmapHeader(m_tamanho, m_tamanho, pixels.size(), 8);
    bmp.insert(bmp.end(), pixels.begin(), pixels.end());
    return bmp;
}

// Library code attributed to this file:
//   wasm func 3661  std::vector<uebyte>::insert(const_iterator pos, size_type n, const uebyte& valor)
//                   (the fill-insert used for the quiet zones above).

} // namespace api
