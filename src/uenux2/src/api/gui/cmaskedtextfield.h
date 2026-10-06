// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cmaskedtextfield.h (path inferred; ctelasvota.cpp of unit u07 includes it).
// Draw (12593 / 12708) was reconstructed by unit u15 in cinputfield.u15.h / cframedtext.u15.h.
//
// api::CMaskedTextField<MASK> : api::IFormField<api::IScreen> - display-only field that shows a text through a
// "mask" (the row of digit boxes, CFramedText, or its grey variant CGrayedFramedText). On the voter screen it
// echoes the number being typed (CDataText<const std::string& (*)()>(&VotoDigitado)).
//   CMaskedTextField<CFramedText>        typeinfo 1537956, vtable @1537916
//   CMaskedTextField<CGrayedFramedText>  typeinfo 1537320, vtable @1537280
// Layout (60 bytes): IFormFieldBase<IScreen> (24) + MASK m_mascara (+24, 28 bytes, own vptr) +
//                    SharedIText m_texto (+52/+56)
// Slots 7/8/9 (GetClassName / Rect / Move) are the shared bodies 6507 / 6505 / 6504 (other unit).
#pragma once

#include <utility>

#include "api/gui/cframedtext.u15.h"
#include "api/gui/iformfield.h"
#include "api/gui/itext.h"

namespace api {

template <class MASK>
class CMaskedTextField : public IFormField<IScreen> {
public:
    CMaskedTextField(MASK mascara, SharedIText texto) : m_mascara(std::move(mascara)), m_texto(std::move(texto)) {}

    // wasm func 12604 (MASK = CFramedText) / 12719 (CGrayedFramedText) - slot 0: thunk into api_f6063:
    //   release m_texto (+56 control block), store the IFormFieldBase<IScreen> vptr, free m_nome.
    //   (CFramedText's destructor is trivial, so nothing is emitted for m_mascara.)
    // wasm func 12595 / 12713 - slot 1: thunk into api_f6062 = the same + operator delete.
    ~CMaskedTextField() override = default;

    void Draw(IScreen& tela) const override { m_mascara.MaskText(tela, m_texto->GetText()); }   // 12593 / 12708

private:
    MASK        m_mascara;   // +24
    SharedIText m_texto;     // +52/+56
};

} // namespace api
