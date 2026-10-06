// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cbeepfieldmt.h (path inferred).
//
// api::CBeepFieldMT : api::IFormField<api::IScreenMT>   (typeinfo 1579612, vtable @1579580, 28 bytes)
// n short beeps of the microterminal when its screen is shown ("Eleitor(a) não reconhecido(a)", end of the
// biometric capture ...). Built by CFormBuilderMT::Add<CBeepFieldMT>(n) (func 1072 -> shared body 3890);
// callers pass 1 or 2.
//   +24 int m_quantidade
// Vtable: 0 IFormFieldBase<IScreenMT> dtor (11052)  1 deleting (ICF 2777)  2 Draw (11053)
//         7 GetClassName (ICF 11051)
#pragma once

#include "api/gui/iformfield.h"

namespace api {

class CBeepFieldMT : public IFormField<IScreenMT> {
public:
    explicit CBeepFieldMT(int quantidade) : m_quantidade(quantidade) {}

    void Draw(IScreenMT& tela) const override;                       // wasm 11053

private:
    int m_quantidade;    // +24
};

} // namespace api
