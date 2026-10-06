// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cframedtext.cpp (srclocs cframedtext.cpp:28 and :66).
// Merge into cframedtext.cpp.
#include "api/gui/cframedtext.u15.h"

#include <algorithm>

#include "api/pattern/cpolysingletonlist.h"

namespace api {

// wasm func 1385 (observed executing) - srcloc cframedtext.cpp:28
// Callers: the numeric input builders (2383/2384), CInputMenuField (inlined in 3675),
// CGrayedFramedText (5535), comum::CMenuBase::vf2 and the vote-screen builders.
CFramedText::CFramedText(size_t digitos, const SPoint& pos, const SFont& fonte, ETextAlignment alinhamento)
    : m_digitos(digitos), m_pos(pos), m_fonte(fonte), m_alinhamento(alinhamento)
{
    // Largest glyph of the font (IScreen slot 2)                                        // name inferred
    CPolySingletonList::instance<IScreen>().GetMaxCharSize(m_larguraCaixa, m_alturaCaixa, m_fonte);

    const TPosition espacamento = Espacamento();
    m_larguraCaixa += espacamento * 2;
    m_alturaCaixa += (m_fonte.size <= 9) ? 2 : (m_fonte.size / 10) * 2;

    const TPosition larguraTotal =
        static_cast<TPosition>(espacamento * (m_digitos - 1) + m_digitos * m_larguraCaixa);
    switch (m_alinhamento) {
    case ETextAlignment::Right:  m_pos.x -= larguraTotal;     break;   // value 1
    case ETextAlignment::Center: m_pos.x -= larguraTotal / 2; break;   // value 2
    default: break;                                                     // Left: unchanged
    }
}

SRect CFramedText::CaixaRect(size_t i) const
{
    const TPosition x0 = static_cast<TPosition>(m_pos.x + (m_larguraCaixa + Espacamento()) * i);
    const TPosition x1 = static_cast<TPosition>(x0 + m_larguraCaixa);
    const TPosition y0 = m_pos.y;
    const TPosition y1 = static_cast<TPosition>(m_pos.y + m_alturaCaixa);
    return SRect{std::min(x0, x1), std::min(y0, y1), std::max(x0, x1), std::max(y0, y1)};
}

// wasm func 2780 (observed executing) - srcloc cframedtext.cpp:66
// Draws `texto` one character per box; free boxes are drawn empty. Callers: CInputField<CFramedText>
// (12438), CMaskedTextField<CFramedText> (12593) and CInputMenuField (10897).
void CFramedText::MaskText(IScreen& tela, const std::string& texto) const
{
    if (texto.size() > m_digitos)
        throw CUeGuiError(static_cast<EUeGuiError>(4917), "Texto [" + texto + "] eh grande demais");

    std::string caixas = texto;
    caixas.append(m_digitos - texto.size(), ' ');      // func 1054 = append(n, c)
    for (size_t i = 0; i < caixas.size(); ++i) {
        if (caixas[i] == ' ') {
            tela.FillRect(CaixaRect(i), /*fundo*/ 1);                     // IScreen slot 6
            tela.DrawRect(CaixaRect(i), /*moldura*/ 2, /*espessura*/ 1);  // IScreen slot 9
        } else {
            DesenhaCaracter(i, caixas[i], tela);                          // func 5537
            tela.DrawRect(CaixaRect(i), /*moldura preenchida*/ 3, 1);
        }
    }
}

} // namespace api
