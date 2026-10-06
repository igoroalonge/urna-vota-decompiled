// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfieldmt.h (path
// inferred from ctextfieldmt.cpp, srcloc record :25).
//
// api::CTextFieldMT : api::IFormField<api::IScreenMT>   (typeinfo 1582676, vtable @1582628, 48 bytes)
// A text field of the mesário's micro-terminal ("MT", the small 4 x 40 character display of the terminal
// do mesário; simulador::CWasmScreenMT in the web build). Positions are column/line on that display.
// Vtable: 0 dtor (10933)  1 deleting (10932)  2 Draw (10934)  3/4 base no-ops  5 RedrawIfDirty (4118)
//         6 SetForm (3037)  7 GetClassName NOT overridden: it is the inherited
//         IFormField<IScreenMT>::GetClassName (func 11051, one table slot 3273 shared by all 10 MT field
//         classes), which returns "IFormField<IScreenMT>" - so CFormBuilder names these fields
//         "IFormField<IScreenMT><n>", not "CTextFieldMT<n>". (IFormFieldBase<IScreenMT> slot 7 is pure.)
//         No Rect/Move: IFormField<IScreenMT> stops at slot 7.
#pragma once

#include "api/gui/iformfield.h"
#include "api/gui/ctextsource.h"

namespace api {

class CTextFieldMT : public IFormField<IScreenMT> {
public:
    CTextFieldMT(const SPoint& pos, const SharedIText& texto);           // wasm func 1262 (:25)
    ~CTextFieldMT() override;                                            // wasm func 10933 / 10932

    void Draw(IScreenMT& tela) const override;                           // wasm func 10934

private:
    SPoint              m_pos;                 // +24
    SharedIText         m_texto;               // +28/+32
    mutable SharedIText m_textoAnterior;       // +36/+40 text drawn last time (to blank it out)
    mutable SPoint      m_posAnterior{};       // +44
};

} // namespace api
