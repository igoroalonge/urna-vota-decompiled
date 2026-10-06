// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cgrayedframedtext.cpp (srcloc cgrayedframedtext.cpp:29). The
// constructor (func 5535) is in cgrayedframedtext.u07.cpp. Merge into cgrayedframedtext.cpp.
#include "api/gui/cframedtext.u15.h"

namespace api {

// wasm func 5536 (tools: api_f5536; only caller 5534)                     // name inferred
// Belongs to CFramedText (protected helper) but was only kept out of line for this caller.
void CFramedText::PreencheCaixa(size_t i, TColor cor, IScreen& tela) const
{
    tela.FillRect(CaixaRect(i), cor);                                     // IScreen slot 6
}

// wasm func 5534 (observed executing) - srcloc cgrayedframedtext.cpp:29
// The number boxes of the voting screens: typed digits on a white box with the "filled" frame,
// the remaining boxes grey (colour 5) with the normal frame.
void CGrayedFramedText::MaskText(IScreen& tela, const std::string& texto) const
{
    if (texto.size() > m_digitos)
        throw CUeGuiError(static_cast<EUeGuiError>(4918), "Texto [" + texto + "] eh grande demais");

    size_t i = 0;
    for (; i < texto.size(); ++i) {
        PreencheCaixa(i, 1, tela);                // white
        DesenhaCaracter(i, texto[i], tela);       // func 5537
        DesenhaMoldura(i, 3, tela);               // func 2779
    }
    for (; i < m_digitos; ++i) {
        PreencheCaixa(i, 5, tela);                // grey
        DesenhaMoldura(i, 2, tela);
    }
}

// wasm func 12708 (observed executing): CMaskedTextField<CGrayedFramedText>::Draw - see
// cframedtext.u15.h (m_mascara.MaskText(tela, m_texto->GetText()), the text copied to a temporary).

} // namespace api
