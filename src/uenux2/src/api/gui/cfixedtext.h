// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cfixedtext.h (path inferred; ctelasvota.cpp of unit u07 already includes it).
//
// api::CFixedText : api::IText   (typeinfo 1532664, vtable @1532648, 20 bytes)
//   +0 vptr   +4 ETextAlignment m_alinhamento   +8 std::string m_texto
// The constant text of labels ("CONFIRMA: prosseguir", "Digite o ANO de nascimento:", "Seq:", ...). It is by
// far the most constructed IText: the vtable is stored by 39 functions. 6 of them use make_shared<CFixedText>
// (control block @1578304: funcs 202, 1265, 2384, 2782, 5546, 7787), 2 use shared_ptr<IText>(new CFixedText)
// (control block @1580048: funcs 180 and the paper text helper shared_f193), and several fields build a
// temporary CFixedText on the stack to hand a computed
// string to the device (CTextFieldUpdateMT::Draw, CInputFieldMT::Draw, CFormBuilder::DesenhaModoUrna ...).
// Every construction site COPIES the text into m_texto (12-byte SSO copy or __init_copy_ctor_external), even
// when the argument is a temporary (10548), so the constructor takes the string by const reference.
#pragma once

#include <string>
#include <utility>

#include "api/gui/itext.h"

namespace api {

class CFixedText : public IText {
public:
    explicit CFixedText(const std::string& texto) : IText(ETextAlignment::Left), m_texto(texto) {}
    CFixedText(ETextAlignment alinhamento, const std::string& texto) : IText(alinhamento), m_texto(texto) {}

    // wasm func 7706 - slot 0: thunk `return shared_f1727(this, vtable CFixedText)`; the merged body stores
    //                  the vptr and frees m_texto (+8) when it is a long string.
    // wasm func 7666 - slot 1 (deleting): thunk into shared_f1969 = the same + operator delete.
    // (1727/1969 are wasm-opt "merge-similar-functions" bodies shared with other {vptr, int, std::string}
    // classes; the vtable is passed as a parameter.)
    ~CFixedText() override = default;

    // slot 2 - ICF body 1139 (other unit): returns a copy of m_texto.
    std::string GetText() const override { return m_texto; }

private:
    std::string m_texto;    // +8
};

} // namespace api
