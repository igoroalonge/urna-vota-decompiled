// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cbmpconversor.cpp (srclocs :23, :27 VerticalFlip, :60 InvertColors;
// CreateBitmapHeader (:122) is in another unit). Merge into cbmpconversor.cpp.
//
// Used on the raw fingerprint image before it is shown/saved: callers are the ProcessTick of
// comum::CPedeDigitalMesario (poll worker), vota::CRegistraDigitalOperador and vota::CPedeDigital (voter).
#include <cstring>
#include <vector>

#include "api/gui/gui-common.u15.h"

namespace api {

class CBmpConversor {
public:
    static void VerticalFlip(unsigned char* dados, const int largura, const int altura);
    static void InvertColors(unsigned char* dados, const int largura, const int altura);
};

// wasm func 3664 (not observed) - srclocs cbmpconversor.cpp:23 and :27
// Swaps row i with row (altura - 1 - i); `largura` is the row size in bytes.
void CBmpConversor::VerticalFlip(unsigned char* dados, const int largura, const int altura)
{
    if (dados == nullptr)
        throw CUeGuiError(static_cast<EUeGuiError>(4900), "Dado de entrada nulo");        // :23
    if (largura * altura < 0)
        throw CUeGuiError(static_cast<EUeGuiError>(4901), "Dimensões inválidas");         // :27

    std::vector<unsigned char> linha(largura);        // throws length_error if largura < 0
    for (int i = 0; i < altura / 2; ++i) {
        unsigned char* cima = dados + largura * i;
        unsigned char* baixo = dados + (altura - (i + 1)) * largura;
        std::memcpy(linha.data(), cima, largura);
        std::memcpy(cima, baixo, largura);
        std::memcpy(baixo, linha.data(), largura);
    }
}

// wasm func 3663 (not observed) - srcloc cbmpconversor.cpp:60
void CBmpConversor::InvertColors(unsigned char* dados, const int largura, const int altura)
{
    if (dados == nullptr)
        throw CUeGuiError(static_cast<EUeGuiError>(4903), "Dado de entrada nulo");        // :60
    const int total = largura * altura;               // no overflow check (see VerticalFlip)
    for (int i = 0; i < total; ++i)
        dados[i] = static_cast<unsigned char>(~dados[i]);
}

} // namespace api
