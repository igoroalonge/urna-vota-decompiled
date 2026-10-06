// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextrectfield.cpp
// (srcloc records :51 constructor, :71 Rect).
#include "api/gui/ctextrectfield.h"

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// (inlined into wasm func 5546 = CFormBuilder::AddTextRect; srcloc line 51)
CTextRectField::CTextRectField(const SRect& area, TColor corFundo, const SharedIText& texto, const SFont& fonte,
                               TColor corTexto)
    : m_area(area), m_corFundo(corFundo), m_texto(texto), m_fonte(fonte), m_corTexto(corTexto)
{
    if (!m_texto)                                                                         // :51
        throw CUeGuiError(EUeGuiError{4976}, "Campo estava com o texto nulo");
}

// wasm func 10904 (D1) / 10903 (D0), bodies 6022 / 6021 shared with CTextFieldDoubleLine.
CTextRectField::~CTextRectField() = default;

// wasm func 10906 (slot 2)
void CTextRectField::Draw(IScreen& tela) const
{
    if (m_rectAnterior.right != m_rectAnterior.left)
        tela.FillRect(m_rectAnterior, m_fundoOpaco ? m_corFundo : TColor{1});             // IScreen slot 6
    m_rectAnterior = tela.WriteText(m_area, *m_texto, m_fonte, m_corTexto,               // IScreen slot 20
                                    m_fundoOpaco ? m_corFundo : TColor{0});
}

// wasm func 10902 (slot 7)
std::string CTextRectField::GetClassName() const
{
    return "CTextRectField";
}

// wasm func 10905 (slot 8, srcloc line 71). Only the extent of the text measured from the top-left corner
// of the band (alignment and band size are ignored).
SRect CTextRectField::Rect() const
{
    IScreen& tela = IScreen::GetInst();                                                  // :71
    TPosition largura = 0, altura = 0;
    tela.GetFontMetrics(largura, altura, m_fonte, m_texto->GetText());
    return SRect(SPoint{m_area.left, m_area.top},
                 SPoint{static_cast<TPosition>(m_area.left + largura), static_cast<TPosition>(m_area.top + altura)});
}

} // namespace api
