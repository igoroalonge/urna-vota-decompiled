// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cnewlinefieldpaper.h (path inferred).
//
// api::CNewLineFieldPaper : api::IFormField<api::IPaper>   (typeinfo 1580940, vtable @1580908, 28 bytes)
// n blank lines on a printed report. Built by CPaperFormBuilder::AddNewLine(n) (func 198 -> shared body 3890),
// the most used paper field (BU, zerésima, RDV extract ...). Lines fed before a cut (CCutFieldPaper) vary:
// 20 in the BU and report trailers (12110, 12105, 5579), 2 in CRelVotaUtil::CortaPapel (2882), 5596 and
// 11908, 1 and 8 in 5591.
//   +24 int m_linhas
// Vtable: 0 IFormFieldBase<IPaper> dtor (11013)  1 deleting (ICF 5516)  2 Draw (11012)
//         7 GetClassName (ICF 11014: "IFormField<IPaper>")
#pragma once

#include "api/gui/iformfield.h"

namespace api {

class CNewLineFieldPaper : public IFormField<IPaper> {
public:
    explicit CNewLineFieldPaper(int linhas) : m_linhas(linhas) {}

    void Draw(IPaper& papel) const override;                         // wasm 11012

private:
    int m_linhas;    // +24
};

} // namespace api
