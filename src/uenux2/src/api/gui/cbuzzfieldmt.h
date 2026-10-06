// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cbuzzfieldmt.h (path inferred).
//
// api::CBuzzFieldMT : api::IFormField<api::IScreenMT>   (typeinfo 1579792, vtable @1579760, 32 bytes)
// Sounds the microterminal buzzer when its screen is shown (e.g. "O eleitor está demorando", "Registrar
// mesários?", biometric capture). Built by CFormBuilderMT::Add<CBuzzFieldMT>(a, b) (func 941); every caller
// passes (51, 10) or (52, 5).
//   +24 int m_param1   +28 int m_param2   (meaning of the two numbers not established: tone/pattern and
//                                          duration/repetitions?)                                        ?
// Vtable: 0 IFormFieldBase<IScreenMT> dtor (11052)  1 deleting (ICF 2777)  2 Draw (11049)
//         7 GetClassName (ICF 11051)
#pragma once

#include "api/gui/iformfield.h"

namespace api {

class CBuzzFieldMT : public IFormField<IScreenMT> {
public:
    CBuzzFieldMT(int param1, int param2) : m_param1(param1), m_param2(param2) {}

    void Draw(IScreenMT& tela) const override;                       // wasm 11049

private:
    int m_param1;    // +24  ?
    int m_param2;    // +28  ?
};

} // namespace api
