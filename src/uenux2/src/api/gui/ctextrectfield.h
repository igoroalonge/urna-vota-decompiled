// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextrectfield.h (path
// inferred from ctextrectfield.cpp, srcloc records :51, :71).
//
// api::CTextRectField : api::IFormField<api::IScreen>   (typeinfo 1583224, vtable @1583152, 68 bytes)
// A text written inside a coloured band. The only use in VOTA is the two yellow (#ffd300) full-width bands
// "Número de cópias" / "acima do limite permitido" ({0,160}-{640,195} and {0,210}-{640,245}, font 35),
// built by vota::CTelasVota's constructor (func 7787) through CFormBuilder::AddTextRect (func 5546).
// Vtable: 0 dtor (10904 -> shared 6022)  1 deleting (10903 -> shared 6021)  2 Draw (10906)
//         7 GetClassName (10902)  8 Rect (10905)  9 Move (shared body 2783)
#pragma once

#include <string>

#include "api/gui/iformfield.h"
#include "api/gui/ctextsource.h"

namespace api {

class CTextRectField : public IFormField<IScreen> {
public:
    CTextRectField(const SRect& area, TColor corFundo, const SharedIText& texto, const SFont& fonte,
                   TColor corTexto);                     // srcloc :51, inlined into wasm func 5546
    ~CTextRectField() override;                          // wasm func 10904 / 10903

    void Draw(IScreen& tela) const override;             // wasm func 10906
    std::string GetClassName() const override;           // wasm func 10902
    SRect Rect() const override;                         // wasm func 10905 (:71)
    void Move(const SPoint& pos) override;               // shared body 2783 (other unit)

private:
    SRect         m_area;                  // +24
    TColor        m_corFundo;              // +32
    SharedIText   m_texto;                 // +36/+40
    SFont         m_fonte;                 // +44
    TColor        m_corTexto;              // +52
    mutable SRect m_rectAnterior{};        // +56
    bool          m_fundoOpaco = true;     // +64 paint m_corFundo behind the text (else transparent)
};

} // namespace api
