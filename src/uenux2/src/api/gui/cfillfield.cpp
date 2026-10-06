// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cfillfield.cpp (path inferred).
#include "api/gui/cfillfield.h"

#include "api/gui/iscreen.h"

namespace api {

// wasm func 11142 (slot 2)
void CFillField::Draw(IScreen& tela) const
{
    tela.FillRect(m_rect, m_cor);             // IScreen slot 6 (CWasmScreen: js_fill)
}

// wasm func 11141 (slot 7)
std::string CFillField::GetClassName() const
{
    return "CFillField";
}

// ICF 2783 (not in this unit)
void CFillField::Move(const SPoint& pos)
{
    if (pos == SPoint{m_rect.left, m_rect.top})
        return;
    m_rect.MoveTo(pos);
    Invalidate();
}

} // namespace api
