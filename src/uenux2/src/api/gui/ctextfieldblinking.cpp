// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfieldblinking.cpp
// (srcloc records :39, :42 constructor, :70 Rect).
#include "api/gui/ctextfieldblinking.h"

#include <chrono>

namespace api {

using namespace std::chrono_literals;
// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// (no out-of-line copy: inlined into wasm func 1265, CFormBuilder::AddBlinkingText; srcloc lines 39, 42)
CTextFieldBlinking::CTextFieldBlinking(const SPoint& pos, const SharedIText& texto, const SFont& fonte,
                                       TColor cor1, TColor cor2, TColor corFundo)
    : m_pos(pos),
      m_texto(texto),
      m_fonte(fonte),
      m_cor1(cor1),
      m_cor2(cor2),
      m_corFundo(corFundo),
      // ITimerScheduler slot 0 (simulador::CWasmTimerScheduler::vf0 in the web build). The lambda is
      // std::function<void()> vtable @1582564, operator() = wasm func 10937.
      m_timer(ITimerScheduler::GetInst().CreateTimer(500ms, [this] {
          m_fase = !m_fase;
          Invalidate();
      }))
{
    if (!m_texto)                                                                         // :39
        throw CUeGuiError(EUeGuiError{4965}, "Campo estava com o texto nulo");
    if (m_cor1 == m_cor2)                                                                 // :42
        throw CUeGuiError(EUeGuiError{4966}, "As cores estavam iguais");
}

// wasm func 10942 (D1) / 10941 (D0): releases m_timer (+60), m_texto (+32), m_nome.
CTextFieldBlinking::~CTextFieldBlinking() = default;

// wasm func 10946 (slot 2). Observed executing. The text is not erased first: both colours paint the
// same glyphs on the same background.
void CTextFieldBlinking::Draw(IScreen& tela) const
{
    tela.WriteText(m_pos, *m_texto, m_fonte, m_fase ? m_cor1 : m_cor2, m_corFundo);   // IScreen slot 19
}

// wasm func 10945 (slot 3). Observed executing (when the form is shown).
void CTextFieldBlinking::Start()
{
    m_timer->Start();                                                                    // ITimer slot 2
}

// wasm func 10944 (slot 4)
void CTextFieldBlinking::Stop()
{
    m_timer->Stop();                                                                     // ITimer slot 3
}

// wasm func 10940 (slot 7). Thunk into wasm func 3891, a merge-similar body that builds an 18-character
// std::string from three pieces (8 + 8 + 2 bytes).
std::string CTextFieldBlinking::GetClassName() const
{
    return "CTextFieldBlinking";
}

// wasm func 10943 (slot 8, srcloc line 70): same body as CTextField::Rect (shared wasm func 3889).
SRect CTextFieldBlinking::Rect() const
{
    IScreen& tela = IScreen::GetInst();                                                  // :70
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
