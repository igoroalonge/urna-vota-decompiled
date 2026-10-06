// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/crectfield.h (path inferred; ctelasvota.cpp of unit u07 includes it).
//
// api::CRectField : api::IFormField<api::IScreen>   (typeinfo 1582220, vtable @1582180, 36 bytes)
// The outline of a rectangle (1-pixel frame) on the voter screen: frames of the candidate photos, the
// separator line of the voting screen (a 2-pixel-high rectangle at y = 400/401), the boxes of the
// accessibility and keyboard-test screens.
//   +24 SRect  m_rect
//   +32 TColor m_cor    (the constructor, func 5501, stores 2 = black; unit u02 calls this member
//                        "m_espessura", but Draw passes it as the COLOUR argument of IScreen::DrawRect)
// Vtable: 0 IFormFieldBase<IScreen> dtor (12658)  1 deleting (ICF 3681)  2 Draw (10960)  3/4 no-op
//         5 RedrawIfDirty (4118)  6 SetForm (3037)  7 GetClassName (10959)  8 Rect (ICF 5551: m_rect)
//         9 Move (ICF 2783: moves m_rect, Invalidate)
#pragma once

#include <string>

#include "api/gui/iformfield.h"

namespace api {

class CRectField : public IFormField<IScreen> {
public:
    explicit CRectField(const SRect& rect) : m_rect(rect) {}          // func 5501 (unit u02, cformbuilder)

    void Draw(IScreen& tela) const override;                         // wasm 10960
    std::string GetClassName() const override;                       // wasm 10959
    SRect Rect() const override { return m_rect; }                   // ICF 5551
    void Move(const SPoint& pos) override;                           // ICF 2783

private:
    SRect  m_rect;       // +24
    TColor m_cor = 2;    // +32
};

} // namespace api
