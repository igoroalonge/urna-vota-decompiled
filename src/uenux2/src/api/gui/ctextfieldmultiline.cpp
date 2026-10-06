// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfieldmultiline.cpp
// (srcloc records :71 constructor, :116 GetTextLines).
#include "api/gui/ctextfieldmultiline.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// wasm func 5498 (srcloc line 71)
CTextFieldMultiLine::CTextFieldMultiLine(const SRect& area, const SharedIText& texto, const SFont& fonte,
                                         TColor corTexto, TColor corFundo)
    : m_area(area),
      m_texto(texto),
      m_fonte(fonte),
      m_corTexto(corTexto),
      m_corFundo(corFundo),
      m_alturaLinha(static_cast<TPosition>(std::ceil(fonte.size / 6.0) + fonte.size))
{
    if (!m_texto)                                                                         // :71
        throw CUeGuiError(EUeGuiError{4972}, "Campo estava com o texto nulo");
}

// wasm func 10923 (D1) / 10922 (D0)
CTextFieldMultiLine::~CTextFieldMultiLine() = default;

// wasm func 5497 (srcloc line 116)
std::vector<std::string> CTextFieldMultiLine::GetTextLines() const
{
    const std::string texto = m_texto->GetText();
    std::vector<std::string> linhas;
    IScreen& tela = IScreen::GetInst();                                                  // :116
    std::istringstream stream(texto);
    const TPosition larguraMaxima = m_area.right - m_area.left + 1;
    const auto largura = [&](const std::string& s) { return tela.GetTextWidth(s, m_fonte); };   // slot 32

    std::string linha;
    while (std::getline(stream, linha)) {
        if (linha.empty()) {                    // blank line of the source text
            linhas.push_back(linha);
            continue;
        }
        do {
            std::size_t quebra = std::string::npos;          // npos: the whole rest fits
            if (largura(linha) > larguraMaxima) {
                // last blank such that everything before it fits
                for (auto espaco = linha.find(' '); espaco != std::string::npos;
                     espaco = linha.find(' ', espaco + 1)) {
                    if (largura(linha.substr(0, espaco)) > larguraMaxima)
                        break;
                    quebra = espaco;
                }
                // no such blank: break inside the word, after the longest prefix that fits
                if (quebra == std::string::npos && linha.size() >= 2) {
                    for (std::size_t n = 1;; ++n) {
                        if (largura(linha.substr(0, n)) > larguraMaxima) {
                            quebra = n - 1;
                            break;
                        }
                        if (n + 1 >= linha.size())
                            break;               // every proper prefix fits: the whole line is kept although
                                                 // it was measured too wide (overflows by its last character)
                    }
                }
            }
            linhas.push_back(linha.substr(0, quebra));
            if (quebra == std::string::npos)
                linha.clear();
            else if (quebra == 0)
                return linhas;                   // an empty line was just pushed; the rest of this paragraph and
                                                 // all following paragraphs are dropped. Reached when the first
                                                 // character does not fit, or when the line starts with a blank
                                                 // followed by a word that does not fit (the blank at 0 is the
                                                 // "last fitting blank")
            else
                linha.erase(0, linha[quebra] == ' ' ? quebra + 1 : quebra);
        } while (!linha.empty());
    }
    return linhas;
}

// wasm func 10924 (slot 8). Before the first Draw: the area trimmed to the height of the lines.
SRect CTextFieldMultiLine::Rect() const
{
    if (m_rectDesenhado != SRect{})
        return m_rectDesenhado;
    const auto linhas = GetTextLines();
    const TPosition fundo = std::min<TPosition>(m_area.top + m_alturaLinha * static_cast<TPosition>(linhas.size()),
                                                m_area.bottom);
    return SRect(SPoint{m_area.left, m_area.top}, SPoint{m_area.right, fundo});
}

// wasm func 10925 (slot 2)
void CTextFieldMultiLine::Draw(IScreen& tela) const
{
    const auto linhas = GetTextLines();
    TPosition y = m_area.top;
    if (m_desenhado)
        tela.FillRect(m_rectDesenhado, m_corFundo);                                      // IScreen slot 6
    m_rectDesenhado = Rect();                                                            // virtual (slot 8)

    for (const std::string& linha : linhas) {
        if (y + m_alturaLinha > m_area.bottom)
            break;                                          // remaining lines are silently dropped
        const TPosition largura = tela.GetTextWidth(linha, m_fonte);                     // IScreen slot 32
        TPosition x;
        switch (m_texto->GetAlignment()) {
        case ETextAlignment::Right:  x = m_area.right - largura;                                break;
        case ETextAlignment::Center: x = m_area.left + (m_area.right - (largura + m_area.left)) / 2; break;
        default:                     x = m_area.left;                                           break;
        }
        const SRect escrito = tela.WriteText(SPoint{x, y}, CFixedText(ETextAlignment::Left, linha), m_fonte,
                                             m_corTexto, m_corFundo);                    // IScreen slot 19
        m_rectDesenhado = Union(m_rectDesenhado, escrito);                               // func 1916
        y += m_alturaLinha;
    }
    m_desenhado = true;
}

// wasm func 10921 (slot 7)
std::string CTextFieldMultiLine::GetClassName() const
{
    return "CTextFieldMultiLine";
}

} // namespace api
