// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfieldupdate.cpp
// (srcloc records :38 constructor, :73 Rect).
#include "api/gui/ctextfieldupdate.h"

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// wasm func 3658 (srcloc line 38). Observed executing (status header of every screen).
CTextFieldUpdate::CTextFieldUpdate(const SPoint& pos, const SharedIText& texto,
                                   const std::chrono::milliseconds& periodo, const SFont& fonte,
                                   TColor corTexto, TColor corFundo)
    : m_pos(pos),
      m_texto(texto),
      m_fonte(fonte),
      m_corTexto(corTexto),
      m_corFundo(corFundo),
      // lambda: std::function<void()> vtable @1583088, operator() = wasm func 10909
      m_timer(ITimerScheduler::GetInst().CreateTimer(periodo, [this] {
          if (m_texto->GetText() != m_ultimoTexto)
              Invalidate();
      }))
{
    if (!m_texto)                                                                         // :38
        throw CUeGuiError(EUeGuiError{4974}, "Campo estava com o texto nulo");
}

// wasm func 5495 (D1) / 10917 (D0 = D1 + free): releases m_timer, m_ultimoTexto, m_texto, m_nome.
CTextFieldUpdate::~CTextFieldUpdate() = default;

// wasm func 10916 (slot 2). Observed executing.
void CTextFieldUpdate::Draw(IScreen& tela) const
{
    if (m_rectAnterior.right != m_rectAnterior.left)        // something was painted before: erase it
        tela.FillRect(m_rectAnterior, m_corFundo);                                       // IScreen slot 6
    m_rectAnterior = tela.WriteText(m_pos, *m_texto, m_fonte, m_corTexto, m_corFundo);   // IScreen slot 19
    m_ultimoTexto = m_texto->GetText();
}

// wasm func 10915 (slot 3)
void CTextFieldUpdate::Start()
{
    m_timer->Start();
}

// wasm func 10914 (slot 4)
void CTextFieldUpdate::Stop()
{
    m_timer->Stop();
}

// wasm func 10912 (slot 7)
std::string CTextFieldUpdate::GetClassName() const
{
    return "CTextFieldUpdate";
}

// wasm func 10913 (slot 8, srcloc line 73). Observed executing. Same body as CTextField::Rect (3889).
SRect CTextFieldUpdate::Rect() const
{
    IScreen& tela = IScreen::GetInst();                                                  // :73
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
