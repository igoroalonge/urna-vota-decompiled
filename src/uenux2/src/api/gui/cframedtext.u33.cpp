// uenux2/src/api/gui/cframedtext.cpp  -- FRAGMENT written by unit u33 (srcloc cframedtext.cpp exists; the class
// is declared in cframedtext.u15.h by unit u15, whose member names are used here).
//
// CFramedText draws the row of digit boxes of the voter screens (candidate number, título...). Box i spans
//   x = m_pos.x + (m_larguraCaixa + espacamento) * i  ..  + m_larguraCaixa,     y = m_pos.y .. + m_alturaCaixa
// with espacamento = (fonte.size <= 7) ? 1 : fonte.size / 8. Rectangles are built with SRect(SPoint, SPoint),
// which normalises the corners with min/max (the i16 select pairs in the binary).
#include "api/gui/cframedtext.u15.h"
#include "api/gui/iscreen.h"

namespace api {

// wasm func 2779 (observed executing: every digit box of the voting screen)        // name inferred
// Callers: CGrayedFramedText::MaskText (5534), CInputMenuField::Draw (10897), CInputField<CFramedText>::Draw
// (12438).
void CFramedText::DesenhaMoldura(size_t i, TColor cor, IScreen& tela) const
{
    const auto esquerda = static_cast<TPosition>(m_pos.x + (m_larguraCaixa + Espacamento()) * i);
    const SRect caixa(SPoint{esquerda, m_pos.y},
                      SPoint{static_cast<TPosition>(esquerda + m_larguraCaixa),
                             static_cast<TPosition>(m_pos.y + m_alturaCaixa)});
    tela.DrawRect(caixa, cor, 1);                                // IScreen slot 9
}

// wasm func 3677                                                                   // name inferred
// Bounding rectangle of all the boxes. Callers: CInputMenuField::Rect (10896), CInputField<CFramedText>::Rect
// (12405) and ICF 6505.
SRect CFramedText::Rect() const
{
    const auto largura = static_cast<TPosition>((m_digitos - 1) * Espacamento() + m_digitos * m_larguraCaixa);
    return SRect(m_pos, SPoint{static_cast<TPosition>(m_pos.x + largura),
                               static_cast<TPosition>(m_pos.y + m_alturaCaixa)});
}

} // namespace api
