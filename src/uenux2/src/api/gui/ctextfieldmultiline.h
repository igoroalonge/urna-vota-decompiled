// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfieldmultiline.h
// (path inferred from ctextfieldmultiline.cpp, srcloc records :71, :116).
//
// api::CTextFieldMultiLine : api::IFormField<api::IScreen>   (typeinfo 1582908, vtable @1582836, 72 bytes)
// A paragraph wrapped inside a rectangle (explicit '\n' honoured, words wrapped, over-long words broken).
// Lines that do not fit vertically are not drawn. Used by ctelasvota.cpp (func 2782, e.g. instruction
// texts) and by vota::CMostraQRCodeBU::StartState.
// Vtable: 0 dtor (10923)  1 deleting (10922)  2 Draw (10925)  7 GetClassName (10921)  8 Rect (10924)
//         9 Move (shared body 2783)
#pragma once

#include <string>
#include <vector>

#include "api/gui/iformfield.h"
#include "api/gui/ctextsource.h"

namespace api {

class CTextFieldMultiLine : public IFormField<IScreen> {
public:
    // srcloc :71. Colours constant-propagated to 2 (black) on 1 (white).
    CTextFieldMultiLine(const SRect& area, const SharedIText& texto, const SFont& fonte,
                        TColor corTexto = TColor{2}, TColor corFundo = TColor{1});   // wasm func 5498
    ~CTextFieldMultiLine() override;                                     // wasm func 10923 / 10922

    void Draw(IScreen& tela) const override;                             // wasm func 10925
    std::string GetClassName() const override;                           // wasm func 10921
    SRect Rect() const override;                                         // wasm func 10924
    void Move(const SPoint& pos) override;                               // shared body 2783 (other unit)

private:
    std::vector<std::string> GetTextLines() const;                       // wasm func 5497 (:116)

    SRect         m_area;                  // +24
    mutable SRect m_rectDesenhado{};       // +32 union of everything painted so far
    mutable bool  m_desenhado = false;     // +40
    SharedIText   m_texto;                 // +44/+48
    SFont         m_fonte;                 // +52
    TColor        m_corTexto;              // +60
    TColor        m_corFundo;              // +64
    TPosition     m_alturaLinha;           // +68 = size + ceil(size / 6)
};

} // namespace api
