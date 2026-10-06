// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/cqrcodeimagepaper.cpp
// (srcloc records :35, :38, :49). Callers: vota::CGeraBU::vf2 (the BU QR codes, via
// comum::CGeradorBUQRCode) and vota_f5591 (certificate QR codes).
#include "api/gui/cqrcodeimagepaper.h"

#include <algorithm>
#include <format>

#include "api/gui/cqrcodeguard.h"
#include "api/util/cstringutils.h"   // api::CStringUtils::SetBit (cstringutils.cpp:83, inlined)

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// wasm func 2772
CQRCodeImagePaper::QrcodeData CQRCodeImagePaper::MontaImagem(std::size_t larguraMaxima, const std::string& texto)
{
    QrcodeData resultado;

    if (texto.empty())                                                                    // :35
        throw CUeGuiError(EUeGuiError{4949}, "Criação de QRCode com texto vazio.");

    // :38 - a third check existed here (its std::source_location record is in the binary) but no code
    // references it: it was removed by constant propagation of larguraMaxima = 400 (probably a
    // "larguraMaxima == 0" test, error 4950?).                                             // ?

    const CQRCodeGuard qr(texto);                                                         // func 5502
    const std::size_t modulos = qr->width;
    const std::size_t largura = modulos + 4;                     // 2 white modules on every side
    if (largura > larguraMaxima)                                                          // :49
        throw CUeGuiError(EUeGuiError{4951},
                       std::format("A largura do qrcode ({}) superou o espaço disponível ({})", largura,
                                   larguraMaxima));

    const std::size_t escala = larguraMaxima / largura;
    std::vector<uebyte> bits((largura * largura + 7) / 8, 0);
    for (std::size_t i = 0; i < modulos * modulos; ++i) {
        if (qr->data[i] & 1) {
            const std::size_t linha = i / modulos;
            const std::size_t coluna = i % modulos;
            // cstringutils.cpp:83: throws CBaseError<EUeUtilError>(7027,
            //   "Bit fora dos limites ({}) do vetor ({})") when bit / 8 >= bits.size()
            CStringUtils::SetBit(bits, coluna + (linha + 2) * largura + 2, true);
        }
    }

    const QrcodeData dados{bits, largura, static_cast<uebyte>(std::min<std::size_t>(escala, 4))};
    resultado = dados;           // copy-assignment (vector assign, func 1681)
    return resultado;            // ~CQRCodeGuard -> QRcode_free (inlined)
}

} // namespace api
