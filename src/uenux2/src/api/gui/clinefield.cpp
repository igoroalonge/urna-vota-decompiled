// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/clinefield.cpp (path inferred).
#include "api/gui/clinefield.h"

#include <algorithm>

#include "api/gui/iscreen.h"

namespace api {

// wasm func 11068 (slot 2)
void CLineField::Draw(IScreen& tela) const
{
    tela.DrawLine(m_inicio, m_fim, m_cor, 1);           // IScreen slot 8 (CWasmScreen: js_line)
}

// wasm func 11066 (slot 7)
std::string CLineField::GetClassName() const
{
    return "CLineField";
}

// wasm func 11067 (slot 8): the bounding box of the two end points, normalised.
SRect CLineField::Rect() const
{
    return SRect{std::min(m_inicio.x, m_fim.x), std::min(m_inicio.y, m_fim.y),
                 std::max(m_inicio.x, m_fim.x), std::max(m_inicio.y, m_fim.y)};
}

// ICF 5527 (not in this unit): moves the line so that it starts at `pos`.
void CLineField::Move(const SPoint& pos)
{
    if (pos == m_inicio)
        return;
    m_fim.x = static_cast<TPosition>(m_fim.x + pos.x - m_inicio.x);
    m_fim.y = static_cast<TPosition>(m_fim.y + pos.y - m_inicio.y);
    m_inicio = pos;
    Invalidate();
}

} // namespace api
