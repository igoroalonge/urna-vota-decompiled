// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/crectfield.cpp (path inferred).
#include "api/gui/crectfield.h"

#include "api/gui/iscreen.h"

namespace api {

// wasm func 10960 (slot 2; observed executing: the separator line of every voting screen)
void CRectField::Draw(IScreen& tela) const
{
    tela.DrawRect(m_rect, m_cor, 1);          // IScreen slot 9 (rect, colour, thickness)
}

// wasm func 10959 (slot 7) - the 10 characters are copied as an SSO immediate
std::string CRectField::GetClassName() const
{
    return "CRectField";
}

// ICF 2783 (shared with other rectangle-based fields; not in this unit)
void CRectField::Move(const SPoint& pos)
{
    if (pos == SPoint{m_rect.left, m_rect.top})
        return;
    m_rect.MoveTo(pos);                        // func 1915
    Invalidate();
}

} // namespace api
