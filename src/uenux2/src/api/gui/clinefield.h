// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/clinefield.h (path inferred).
//
// api::CLineField : api::IFormField<api::IScreen>   (typeinfo 1579404, vtable @1579364, 36 bytes)
// A straight line between two points (1 pixel thick). Built by CFormBuilder::AddLine (func 2244), used by
// CMostraQRCodeBU / CImprimirBUOutrasObrigatorias (screens that show the BU QR codes) and the
// accessibility instructions.
//   +24 SPoint m_inicio   +28 SPoint m_fim   +32 TColor m_cor
// Vtable: 0 IFormFieldBase<IScreen> dtor (12658)  1 deleting (ICF 3681)  2 Draw (11068)
//         7 GetClassName (11066)  8 Rect (11067)  9 Move (ICF 5527: translates both points)
#pragma once

#include <string>

#include "api/gui/iformfield.h"

namespace api {

class CLineField : public IFormField<IScreen> {
public:
    CLineField(const SPoint& inicio, const SPoint& fim, TColor cor) : m_inicio(inicio), m_fim(fim), m_cor(cor) {}

    void Draw(IScreen& tela) const override;                         // wasm 11068
    std::string GetClassName() const override;                       // wasm 11066
    SRect Rect() const override;                                     // wasm 11067
    void Move(const SPoint& pos) override;                           // ICF 5527

private:
    SPoint m_inicio;   // +24
    SPoint m_fim;      // +28
    TColor m_cor;      // +32
};

} // namespace api
