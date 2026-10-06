// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cclockfieldmt.h (path inferred).
//
// api::CClockFieldMT : api::IFormField<api::IScreenMT>   (typeinfo 1579844, vtable @1579812, 28 bytes)
// The clock (hh:mm:ss) of the poll worker's microterminal, normally at column 33 of line 1. Built by
// CFormBuilderMT::Add<CClockFieldMT>(pos) (func 728). The device keeps the clock running by itself.
//   +24 SPoint m_pos
// Vtable: 0 IFormFieldBase<IScreenMT> dtor (11052)  1 deleting (ICF 2777)  2 Draw (11048)
//         7 GetClassName (ICF 11051: "IFormField<IScreenMT>")
#pragma once

#include "api/gui/iformfield.h"

namespace api {

class CClockFieldMT : public IFormField<IScreenMT> {
public:
    explicit CClockFieldMT(const SPoint& pos) : m_pos(pos) {}

    void Draw(IScreenMT& tela) const override;                       // wasm 11048

private:
    SPoint m_pos;    // +24
};

} // namespace api
