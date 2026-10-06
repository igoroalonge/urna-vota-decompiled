// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfielddoubleline.cpp
// (srcloc records :40, :43, :46 constructor, :68 GetTextLines, :133 CalcLineRect).
#include "api/gui/ctextfielddoubleline.h"

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// wasm func 3659 (srcloc lines 40, 43, 46). Observed executing.
CTextFieldDoubleLine::CTextFieldDoubleLine(const SPoint& linha1, const SPoint& linha2, TPosition maxX,
                                           const SharedIText& texto, const SFont& fonte, TColor corTexto,
                                           TColor corFundo)
    : m_linha1(linha1),
      m_linha2(linha2),
      m_maxX(maxX),
      m_texto(texto),
      m_fonte(fonte),
      m_corTexto(corTexto),
      m_corFundo(corFundo)
{
    if (!m_texto)                                                                         // :40
        throw CUeGuiError(EUeGuiError{4967}, "Campo estava com o texto nulo");
    if (m_maxX < m_linha1.x)                                                              // :43
        throw CUeGuiError(EUeGuiError{4968}, "maxX menor que a posição x da linha 1");
    if (m_maxX < m_linha2.x)                                                              // :46
        throw CUeGuiError(EUeGuiError{4969}, "maxX menor que a posição x da linha 2");
}

// wasm func 10928 (D1) / 10927 (D0): 12-byte thunks into the bodies shared with CTextRectField (6022 /
// 6021, the only non-trivial member is a shared_ptr at +36).
CTextFieldDoubleLine::~CTextFieldDoubleLine() = default;

// wasm func 5499 (srcloc line 68). Observed executing. Word wrap onto two lines. Widths are measured with
// the real font (IScreen slot 32). The alignment of the text source is not taken into account here.
std::tuple<std::string, std::string> CTextFieldDoubleLine::GetTextLines() const
{
    std::string linha1 = m_texto->GetText();
    std::string linha2;
    IScreen& tela = IScreen::GetInst();                                                  // :68
    const TPosition largura1 = m_maxX - m_linha1.x + 1;

    if (tela.GetTextWidth(linha1, m_fonte) > largura1) {
        auto espaco = linha1.rfind(' ');
        if (espaco != std::string::npos) {
            // 1. move whole words to line 2 while line 1 is too wide
            linha2 = linha1.substr(espaco + 1);
            linha1.resize(espaco);
            bool separar = true;         // next character moved must be followed by a blank
            while (tela.GetTextWidth(linha1, m_fonte) > largura1) {
                espaco = linha1.rfind(' ');
                if (espaco != std::string::npos) {
                    linha2 = linha1.substr(espaco + 1) + " " + linha2;
                    linha1.resize(espaco);
                    separar = true;
                } else {
                    // 2. a single word is still too wide: move it character by character
                    if (separar)
                        linha2 = " " + linha2;
                    linha2 = linha1.back() + linha2;
                    linha1.erase(linha1.size() - 1);          // out_of_range if empty (unreachable)
                    separar = false;
                }
            }
        } else {
            // no blank at all: break the word character by character
            linha2 = linha1.back();
            linha1.erase(linha1.size() - 1);
            while (tela.GetTextWidth(linha1, m_fonte) > largura1) {
                linha2 = linha1.back() + linha2;
                linha1.erase(linha1.size() - 1);
            }
        }
    }

    // 3. line 2 is simply cut at its own limit (no ellipsis)
    if (!linha2.empty()) {
        const TPosition largura2 = m_maxX - m_linha2.x + 1;
        while (tela.GetTextWidth(linha2, m_fonte) > largura2) {
            linha2.erase(linha2.size() - 1);
            if (linha2.empty())
                break;
        }
    }
    return {linha1, linha2};
}

// srcloc line 133 (inlined twice into Rect)
SRect CTextFieldDoubleLine::CalcLineRect(const std::string& linha, SPoint pos) const
{
    IScreen& tela = IScreen::GetInst();                                                  // :133
    TPosition largura = 0, altura = 0;
    tela.GetFontMetrics(largura, altura, m_fonte, linha);
    switch (m_texto->GetAlignment()) {
    case ETextAlignment::Right:  pos.x -= largura;      break;
    case ETextAlignment::Center: pos.x -= largura / 2;  break;
    default:                                            break;
    }
    return SRect(pos, SPoint{static_cast<TPosition>(pos.x + largura), static_cast<TPosition>(pos.y + altura)});
}

// wasm func 10929 (slot 8). The tools named it CalcLineRect after the srcloc record of the inlined helper.
SRect CTextFieldDoubleLine::Rect() const
{
    const auto [linha1, linha2] = GetTextLines();
    SRect rect = CalcLineRect(linha1, m_linha1);
    if (!linha2.empty())
        rect = Union(rect, CalcLineRect(linha2, m_linha2));   // func 1916: min/min/max/max  // name inferred
    return rect;
}

// wasm func 10931 (slot 2). Observed executing. Nothing is erased: the form repaints the background.
void CTextFieldDoubleLine::Draw(IScreen& tela) const
{
    const auto [linha1, linha2] = GetTextLines();
    tela.WriteText(m_linha1, CFixedText(m_texto->GetAlignment(), linha1), m_fonte, m_corTexto, m_corFundo);
    if (!linha2.empty())
        tela.WriteText(m_linha2, CFixedText(m_texto->GetAlignment(), linha2), m_fonte, m_corTexto, m_corFundo);
}

// wasm func 10926 (slot 7)
std::string CTextFieldDoubleLine::GetClassName() const
{
    return "CTextFieldDoubleLine";
}

} // namespace api
