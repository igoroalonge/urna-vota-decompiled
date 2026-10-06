// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfieldpaper.h (path
// inferred from ctextfieldpaper.cpp, srcloc record :23).
//
// api::CTextFieldPaper : api::IFormField<api::IPaper>   (typeinfo 1582976, vtable @1582928, 36 bytes)
// One line of a printed report (BU, zerésima, relatórios): the text source plus a print style. Used by
// vota::CGeraBU (cargo names), vota::CGeraRelatorios::StartState, comum_f604, comum_f5613.
// Vtable: 0 dtor (10919)  1 deleting (10918)  2 Draw (10920)  3/4 base no-ops  5 RedrawIfDirty (4118)
//         6 SetForm (3037)  7 GetClassName NOT overridden: it is the inherited
//         IFormField<IPaper>::GetClassName (func 11014 -> merge-similar body 3891, one table slot 3367 shared
//         with CCutFieldPaper, CNewLineFieldPaper, CQRCodeImageFieldPaper), which returns "IFormField<IPaper>".
//         (IFormFieldBase<IPaper> slot 7 is pure.)
#pragma once

#include "api/gui/iformfield.h"
#include "api/gui/ctextsource.h"
#include "api/gui/ipaper.h"   // api::IPaper, IPaper::EStyle (attested in the srcloc signature)

namespace api {

class CTextFieldPaper : public IFormField<IPaper> {
public:
    CTextFieldPaper(const SharedIText& texto, IPaper::EStyle estilo);     // wasm func 2771 (:23)
    ~CTextFieldPaper() override;                                          // wasm func 10919 / 10918

    void Draw(IPaper& papel) const override;                              // wasm func 10920

private:
    SharedIText    m_texto;    // +24/+28
    IPaper::EStyle m_estilo;   // +32
};

} // namespace api
