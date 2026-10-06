// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfielddoubleline.h
// (path inferred from ctextfielddoubleline.cpp, srcloc records :40, :43, :46, :68, :133).
//
// api::CTextFieldDoubleLine : api::IFormField<api::IScreen>   (typeinfo 1582816, vtable @1582696, 60 bytes)
// A text that wraps onto a second line when it does not fit between its x and `maxX`; whatever does not
// fit on the second line is cut. Used on the voting screens for the cargo name, the candidate name and
// the party name (ctelasvota.cpp: func 3068 Add<CTextFieldDoubleLine>, 1192, 4185).
// Vtable: 0 dtor (10928 -> shared 6022)  1 deleting (10927 -> shared 6021)  2 Draw (10931)
//         7 GetClassName (10926)  8 Rect (10929, CalcLineRect inlined twice)  9 Move (shared body 5527)
#pragma once

#include <string>
#include <tuple>

#include "api/gui/iformfield.h"
#include "api/gui/ctextsource.h"

namespace api {

class CTextFieldDoubleLine : public IFormField<IScreen> {
public:
    // srcloc :40/:43/:46. The text colour is constant-propagated to 2 (black) by the compiler.
    CTextFieldDoubleLine(const SPoint& linha1, const SPoint& linha2, TPosition maxX, const SharedIText& texto,
                         const SFont& fonte, TColor corTexto, TColor corFundo);   // wasm func 3659
    ~CTextFieldDoubleLine() override;                                    // wasm func 10928 / 10927

    void Draw(IScreen& tela) const override;                             // wasm func 10931
    std::string GetClassName() const override;                           // wasm func 10926
    SRect Rect() const override;                                         // wasm func 10929
    void Move(const SPoint& pos) override;                               // shared body 5527 (other unit)

private:
    std::tuple<std::string, std::string> GetTextLines() const;           // wasm func 5499 (:68)
    SRect CalcLineRect(const std::string& linha, SPoint pos) const;      // :133, inlined into Rect

    SPoint      m_linha1;     // +24 start of line 1
    SPoint      m_linha2;     // +28 start of line 2
    TPosition   m_maxX;       // +32 right limit of both lines (inclusive)
    SharedIText m_texto;      // +36/+40
    SFont       m_fonte;      // +44
    TColor      m_corTexto;   // +52
    TColor      m_corFundo;   // +56
};

} // namespace api
