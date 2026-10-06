// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/ccutfieldpaper.h (path inferred).
//
// api::CCutFieldPaper : api::IFormField<api::IPaper>   (typeinfo 1580828, vtable @1580796, 24 bytes)
// "Cut the paper here": ends every printed document (each via of the BU, the zerésima, the reports).
// Built by CPaperFormBuilder::AddCut (func 1264).
// Vtable: 0 IFormFieldBase<IPaper> dtor (11013)  1 deleting (ICF 5516)  2 Draw (11015)
//         7 GetClassName (ICF 11014: "IFormField<IPaper>")
#pragma once

#include "api/gui/iformfield.h"

namespace api {

class CCutFieldPaper : public IFormField<IPaper> {
public:
    void Draw(IPaper& papel) const override;                         // wasm 11015
};

} // namespace api
