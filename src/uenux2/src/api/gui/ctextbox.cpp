// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextbox.cpp
// (srcloc records :41 constructor, :69 Draw, :90 Move, :110 Rect).
#include "api/gui/ctextbox.h"

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// (no out-of-line copy: inlined into wasm func 11805, CTesteTeclado::StartState; srcloc line 41)
CTextBox::CTextBox(const std::string& texto, ETextStatus status, SPoint pos, TFontSize tamanhoFonte,
                   ETextAlignment alinhamento, SPoint tamanho, TColor corFundo, TColor corTexto)
    : m_texto(alinhamento, texto),
      m_status(status),
      m_pos(pos),
      m_tamanhoFonte(tamanhoFonte),
      m_corFundo(corFundo),
      m_corTexto(corTexto),
      m_altura(tamanho.y),
      m_largura(tamanho.x)
{
    if (m_pos.x < 3 || m_pos.y < 1)                                                       // :41
        throw CUeGuiError(EUeGuiError{4961}, "A posição da caixa não pode ter x < 3 ou y < 1.");
}

// wasm func 10955 (D1) / 10954 (D0): destroys the embedded CFixedText, then m_nome.
CTextBox::~CTextBox() = default;

// wasm func 10956 (slot 8, srcloc line 110)
SRect CTextBox::Rect() const
{
    SRect rect;
    if (m_largura != 0 && m_altura != 0) {
        rect = SRect(m_pos, SPoint{static_cast<TPosition>(m_pos.x + m_largura),
                                   static_cast<TPosition>(m_pos.y + m_altura)});
    } else {                                   // size from the text plus 20 x 8 pixels of padding
        IScreen& tela = IScreen::GetInst();                                              // :110
        TPosition largura = 0, altura = 0;
        tela.GetFontMetrics(largura, altura, SFont{m_tamanhoFonte, 0}, m_texto.GetText());
        rect = SRect(m_pos, SPoint{static_cast<TPosition>(m_pos.x + largura + 20),
                                   static_cast<TPosition>(m_pos.y + altura + 8)});
    }
    // m_pos is the left end, the right end or the centre of the box, following the text alignment
    TPosition deslocamento = 0;
    switch (m_texto.GetAlignment()) {
    case ETextAlignment::Right:  deslocamento = -(rect.right - rect.left + 1);     break;
    case ETextAlignment::Center: deslocamento = -((rect.right - rect.left + 1) / 2); break;
    default:                     return rect;
    }
    rect.left += deslocamento;
    rect.right += deslocamento;
    return rect;
}

// wasm func 10957 (slot 9, srcloc line 90). The new position is stored before it is validated.
void CTextBox::Move(const SPoint& pos)
{
    if (pos.x == m_pos.x && pos.y == m_pos.y)
        return;
    m_pos = pos;
    if (m_pos.x < 3 || m_pos.y <= 0)                                                      // :90
        throw CUeGuiError(EUeGuiError{4963}, "A posição da caixa não pode ter x < 3 ou y < 1.");
    Invalidate();
}

// wasm func 10958 (slot 2, srcloc line 69)
void CTextBox::Draw(IScreen& tela) const
{
    const SRect rect = Rect();                                                           // virtual (slot 8)
    const SPoint centro{static_cast<TPosition>(rect.left + (rect.right - rect.left + 1) / 2),
                        static_cast<TPosition>(rect.top + 4)};
    const CFixedText texto(ETextAlignment::Center, m_texto.GetText());
    const SFont fonte{m_tamanhoFonte, 0};

    tela.ClearRect(rect, TColor{1});                                                     // IScreen slot 5
    switch (m_status) {
    case ETextStatus::Normal:
        tela.SetColors(m_corTexto, m_corFundo);                                          // IScreen slot 29 (?)
        tela.DrawRect(rect, m_corTexto, 1);                                              // IScreen slot 9
        tela.WriteText(centro, texto, fonte, m_corTexto, TColor{0});                     // IScreen slot 19
        break;
    case ETextStatus::Invertido:
        tela.SetColors(m_corFundo, m_corTexto);
        tela.FillRect(rect, m_corTexto);                                                 // IScreen slot 6
        tela.WriteText(centro, texto, fonte, m_corFundo, TColor{0});
        break;
    case ETextStatus::Desativado:
        tela.FillRect(rect, TColor{4});                                                  // #d3d3d3
        tela.SetColors(m_corTexto, TColor{4});
        tela.WriteText(centro, texto, fonte, m_corFundo, TColor{0});
        break;
    default:                                                                             // :69
        throw CUeGuiError(EUeGuiError{4962}, "Status indefinido");
    }
    tela.SetColors(m_corTexto, m_corFundo);
}

// wasm func 10953 (slot 7)
std::string CTextBox::GetClassName() const
{
    return "CTextBox";
}

} // namespace api
