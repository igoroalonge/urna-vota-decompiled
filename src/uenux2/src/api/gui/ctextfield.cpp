// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfield.cpp
// (srcloc records :30 constructor, :55 Rect).
#include "api/gui/ctextfield.h"

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// wasm func 1918 (srcloc line 30). Callers: CFormBuilder::AddText (202), Add<CTextField> (1191),
// ctelasvota.cpp helpers (1192, 2028, 6528), vota::CMostraQRCodeBU::StartState.
CTextField::CTextField(const SPoint& pos, const SharedIText& texto, const SFont& fonte, TColor corTexto,
                       TColor corFundo)
    : m_pos(pos), m_texto(texto), m_fonte(fonte), m_corTexto(corTexto), m_corFundo(corFundo)
{
    if (!m_texto)                                                                         // :30
        throw CUeGuiError(EUeGuiError{4964}, "Campo estava com o texto nulo");
}

// wasm func 10950 (D1) / 10948 (D0): releases m_texto, then ~IFormFieldBase (m_nome).
CTextField::~CTextField() = default;

// wasm func 10952 (slot 2). Observed executing.
void CTextField::Draw(IScreen& tela) const
{
    if (m_apagarAnterior)
        tela.FillRect(m_rectAnterior, m_corFundo);                                       // IScreen slot 6
    m_rectAnterior = tela.WriteText(m_pos, *m_texto, m_fonte, m_corTexto, m_corFundo);   // IScreen slot 19
    m_apagarAnterior = false;
}

// wasm func 10947 (slot 7)
std::string CTextField::GetClassName() const
{
    return "CTextField";
}

// wasm func 10951 (slot 8, srcloc line 55). 14-byte thunk into wasm func 3889 (unit u33), the body that
// merge-similar-functions shared between CTextField, CTextFieldBlinking and CTextFieldUpdate (same member
// layout: pos +24, text +28, font +36); the only difference is the srcloc of the IScreen lookup.
SRect CTextField::Rect() const
{
    IScreen& tela = IScreen::GetInst();          // CPolySingletonList::instance<IScreen>(source_location :55)
    TPosition largura = 0, altura = 0;
    tela.GetFontMetrics(largura, altura, m_fonte, m_texto->GetText());
    TPosition x = m_pos.x;
    switch (m_texto->GetAlignment()) {
    case ETextAlignment::Right:  x = m_pos.x - largura;      break;
    case ETextAlignment::Center: x = m_pos.x - largura / 2;  break;
    default:                                                 break;
    }
    return SRect(SPoint{x, m_pos.y},
                 SPoint{static_cast<TPosition>(x + largura), static_cast<TPosition>(m_pos.y + altura)});
}

} // namespace api
