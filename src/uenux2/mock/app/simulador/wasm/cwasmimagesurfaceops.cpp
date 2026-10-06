// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp
//
// simulador::CWasmImageSurfaceOps - image measuring/placing helpers of the web screen (see the header).
// 16 wasm functions (one per vtable slot 2..17). Observed executing: 8998 (GetImageSize(IImage)).
#include "simulador/wasm/cwasmimagesurfaceops.h"

#include <algorithm>
#include <cstring>

namespace simulador {

namespace {

std::uint16_t Be16(const uebyte* p) { return static_cast<std::uint16_t>(p[0] << 8 | p[1]); }
std::uint16_t Le16(const uebyte* p) { return static_cast<std::uint16_t>(p[0] | p[1] << 8); }

// JPEG start-of-frame markers that carry the image size: C0..C3, C5..C7, C9..CB, CD..CF
// (bit mask 0x777 = 1911 over marker - 0xC5 in the binary).
bool EhMarcadorSOF(uebyte m)
{
    if ((m & 0xFC) == 0xC0)
        return true;
    const unsigned d = m - 0xC5u;
    return d <= 10 && ((1u << d) & 0x777u) != 0;
}

}  // namespace

// ------------------------------------------------------------------------------------------------------
// The header parser (wasm func 3493 = slot 12, and an inlined copy in func 8605). Returns {width, height};
// {1, 1} for anything it does not understand.
//   GIF  ("GIF", >= 10 bytes): little-endian 16-bit width/height at offsets 6/8
//   PNG  (8-byte signature, >= 24 bytes): low 16 bits of the big-endian IHDR width/height (offsets 18/22)
//   JPEG (FF D8, >= 12 bytes): walks the markers until a SOFn and reads height/width from it; stops (1 x 1)
//        at EOI/SOS, at a segment length < 2 or at a segment running past the end.
//   BMP is NOT recognised: every "BM" image (all QR codes of the urna, api::CQRCodeImage) measures 1 x 1.
api::SPoint TamanhoImagem(const std::vector<uebyte>& dados)
{
    const std::size_t n = dados.size();
    if (n == 0)
        return {1, 1};
    const uebyte* d = dados.data();

    if (n >= 10 && d[0] == 'G' && d[1] == 'I' && d[2] == 'F')
        return {static_cast<api::TPosition>(Le16(d + 6)), static_cast<api::TPosition>(Le16(d + 8))};

    static constexpr uebyte ASSINATURA_PNG[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    if (n >= 24 && std::memcmp(d, ASSINATURA_PNG, 8) == 0)
        return {static_cast<api::TPosition>(Be16(d + 18)), static_cast<api::TPosition>(Be16(d + 22))};

    if (n < 12 || d[0] != 0xFF || d[1] != 0xD8)
        return {1, 1};

    std::size_t i = 2;
    do {
        if (d[i] != 0xFF) {
            ++i;
            continue;
        }
        if (n < i + 4)
            return {1, 1};
        const uebyte marcador = d[i + 1];
        if (marcador == 0xD9 || marcador == 0xDA)                   // EOI, SOS
            return {1, 1};
        const std::size_t tamanhoSegmento = Be16(d + i + 2);
        if (tamanhoSegmento < 2)
            return {1, 1};
        const std::size_t inicio = i;
        i += 2 + tamanhoSegmento;
        if (i > n)
            return {1, 1};
        if (EhMarcadorSOF(marcador))
            return {static_cast<api::TPosition>(Be16(d + inicio + 7)),    // width
                    static_cast<api::TPosition>(Be16(d + inicio + 5))};   // height
    } while (i + 9 < n);
    return {1, 1};
}

// ------------------------------------------------------------------------------------------------------
// wasm func 9365 - slot 2 (?): a "surface" is a shared copy of the encoded bytes; the size is ignored.
api::SharedSurface CWasmImageSurfaceOps::CreateSurface(const api::IImage& imagem, api::TPosition, api::TPosition)
{
    return std::make_shared<std::vector<uebyte>>(imagem.GetImage());
}

// wasm func 9362 - slot 3 (?): forwards to virtual slot 3 of its first argument.
int CWasmImageSurfaceOps::Slot3(const api::ISurfaceSource& fonte, int, int)
{
    return fonte.Slot3();                                                       // call_indirect(fonte, vtable[3])
}

// wasm func 9356 - slot 4 (?): returns the first word of its first argument.
int CWasmImageSurfaceOps::Slot4(const int& valor, int, int) { return valor; }

// wasm func 9348 - slot 5 (?): same as slot 2 with one extra (ignored) parameter.
api::SharedSurface CWasmImageSurfaceOps::CreateSurface(const api::IImage& imagem, int)
{
    return std::make_shared<std::vector<uebyte>>(imagem.GetImage());
}

// wasm func 9339 - slot 6 (?)
int CWasmImageSurfaceOps::Slot6(const api::ISurfaceSource& fonte, int)
{
    return fonte.Slot3();
}

// wasm func 9333 - slot 7 (?)
int CWasmImageSurfaceOps::Slot7(const int& valor, int) { return valor; }

// wasm func 9328 - slot 8 (?): raw heap copy of the bytes (released by slot 11).
uebyte* CWasmImageSurfaceOps::CopyToBuffer(const std::vector<uebyte>& dados)
{
    auto* buffer = new uebyte[dados.size()];
    if (!dados.empty())
        std::memcpy(buffer, dados.data(), dados.size());
    return buffer;
}

// wasm func 9322 - slot 9 (?): an empty surface.
api::SharedSurface CWasmImageSurfaceOps::CreateEmptySurface(const int&)
{
    return std::make_shared<std::vector<uebyte>>();
}

// wasm func 9317 - slot 10 (?): slot 9 with the value returned by virtual slot 3 of the argument.
api::SharedSurface CWasmImageSurfaceOps::CreateEmptySurface(const api::ISurfaceSource& fonte)
{
    const int parametro = fonte.Slot3();
    return CreateEmptySurface(parametro);                                                   // slot 9
}

// wasm func 9307 - slot 11 (?)
void CWasmImageSurfaceOps::FreeBuffer(uebyte* buffer)
{
    delete[] buffer;
}

// wasm func 3493 - slot 12. Also called directly by CWasmScreen::DrawImage (9079, 9058).
api::SPoint CWasmImageSurfaceOps::GetImageSize(const std::vector<uebyte>& dados)
{
    return TamanhoImagem(dados);
}

// wasm func 8998 - slot 13. Observed executing (CInfoMTLCD).
api::SPoint CWasmImageSurfaceOps::GetImageSize(const api::IImage& imagem)
{
    const std::vector<uebyte> dados = imagem.GetImage();
    return GetImageSize(dados);                                                 // slot 12
}

// wasm func 9289 - slot 14: box of a w x h object anchored at `pos` (logical coordinates; the far edges are
// pos + size, not pos + size - 1).
api::SRect CWasmImageSurfaceOps::CalcRect(api::TPosition largura, api::TPosition altura, const api::SPoint& pos,
                                          api::EAnchorPoint ancora)
{
    short x = pos.x, y = pos.y;
    switch (static_cast<int>(ancora)) {
    case 1: x -= largura;                            break;
    case 2: x += largura / -2;                       break;
    case 3: y += altura / -2;                        break;
    case 4: x -= largura;      y += altura / -2;     break;
    case 5: x += largura / -2; y += altura / -2;     break;
    case 6: y -= altura;                             break;
    case 7: x -= largura;      y -= altura;          break;
    case 8: x += largura / -2; y -= altura;          break;
    default:                                         break;
    }
    const short x2 = static_cast<short>(x + largura), y2 = static_cast<short>(y + altura);
    return {std::min(x, x2), std::min(y, y2), std::max(x, x2), std::max(y, y2)};
}

// wasm func 8994 - slot 15
api::SRect CWasmImageSurfaceOps::GetImageRect(const std::vector<uebyte>& dados, const api::SPoint& pos,
                                              api::EAnchorPoint ancora)
{
    const api::SPoint tamanho = GetImageSize(dados);                            // slot 12
    return CalcRect(tamanho.x, tamanho.y, pos, ancora);                         // slot 14
}

// wasm func 8991 - slot 16 (api::CImageField::Rect, CImageFieldUpdate::Rect)
api::SRect CWasmImageSurfaceOps::GetImageRect(const api::IImage& imagem, const api::SPoint& pos,
                                              api::EAnchorPoint ancora)
{
    const std::vector<uebyte> dados = imagem.GetImage();
    return GetImageRect(dados, pos, ancora);                                    // slot 15
}

// wasm func 8985 - slot 17 (the shared_ptr is taken by value and released by the callee: libc++ ABI v2 marks
// shared_ptr [[clang::trivial_abi]])
api::SRect CWasmImageSurfaceOps::GetImageRect(std::shared_ptr<api::IImage> imagem, const api::SPoint& pos,
                                              api::EAnchorPoint ancora)
{
    const std::vector<uebyte> dados = imagem->GetImage();
    return GetImageRect(dados, pos, ancora);                                    // slot 15
}

}  // namespace simulador
