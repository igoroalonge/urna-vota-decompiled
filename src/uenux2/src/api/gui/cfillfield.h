// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cfillfield.h (path inferred).
//
// api::CFillField : api::IFormField<api::IScreen>   (typeinfo 1577828, vtable @1577788, 36 bytes)
// A filled rectangle (coloured band/background). Built by CFormBuilder::AddFill (func 2245), which
// normalises the two corners (min/max) before storing them.
//   +24 SRect m_rect   +32 TColor m_cor
// Vtable: 0 IFormFieldBase<IScreen> dtor (12658)  1 deleting (ICF 3681)  2 Draw (11142)
//         7 GetClassName (11141)  8 Rect (ICF 5551)  9 Move (ICF 2783)
#pragma once

#include <string>

#include "api/gui/iformfield.h"

namespace api {

class CFillField : public IFormField<IScreen> {
public:
    CFillField(const SRect& rect, TColor cor) : m_rect(rect), m_cor(cor) {}

    void Draw(IScreen& tela) const override;                         // wasm 11142
    std::string GetClassName() const override;                       // wasm 11141
    SRect Rect() const override { return m_rect; }                   // ICF 5551
    void Move(const SPoint& pos) override;                           // ICF 2783 (same body as CRectField::Move)

private:
    SRect  m_rect;   // +24
    TColor m_cor;    // +32
};

} // namespace api
