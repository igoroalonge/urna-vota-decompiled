// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextbox.h (path inferred
// from ctextbox.cpp, srcloc records :41, :69, :90, :110).
//
// api::CTextBox : api::IFormField<api::IScreen>   (typeinfo 1582344, vtable @1582240, 68 bytes)
// A framed box with a centred label and three looks (ETextStatus). The only user in VOTA is the keyboard
// test of the start of the day (vota::testeteclado::CTesteTeclado::StartState, func 11805, which inlines
// the constructor): one box per key of the urna keypad, drawn inverted when the key has been pressed.
// Vtable: 0 dtor (10955)  1 deleting (10954)  2 Draw (10958)  7 GetClassName (10953)  8 Rect (10956)
//         9 Move (10957)
#pragma once

#include <string>

#include "api/gui/iformfield.h"
#include "api/gui/ctextsource.h"

namespace api {

enum class ETextStatus : int {       // attested type name; enumerator names inferred
    Normal     = 0,                  // white box, 1-px outline, text in m_corTexto
    Invertido  = 1,                  // box filled with m_corTexto, text in m_corFundo
    Desativado = 2,                  // box filled with colour 4 (#d3d3d3), text in m_corFundo
};

class CTextBox : public IFormField<IScreen> {
public:
    // srcloc :41. The keyboard test passes (tecla, Normal, pos, tamanhoFonte, Center, tamanho, 1, 2).
    CTextBox(const std::string& texto, ETextStatus status, SPoint pos, TFontSize tamanhoFonte,
             ETextAlignment alinhamento, SPoint tamanho, TColor corFundo, TColor corTexto);   // ? colour order
    ~CTextBox() override;                                                // wasm func 10955 / 10954

    void Draw(IScreen& tela) const override;                             // wasm func 10958 (:69)
    std::string GetClassName() const override;                           // wasm func 10953
    SRect Rect() const override;                                         // wasm func 10956 (:110)
    void Move(const SPoint& pos) override;                               // wasm func 10957 (:90)

private:
    CFixedText  m_texto;          // +24 (vptr +24, alignment +28, std::string +32)
    ETextStatus m_status;         // +44
    SPoint      m_pos;            // +48
    TFontSize   m_tamanhoFonte;   // +52
    TColor      m_corFundo;       // +56  1 (white) in the keyboard test
    TColor      m_corTexto;       // +60  2 (black)
    TPosition   m_altura;         // +64  tamanho.y (0 = from the text)
    TPosition   m_largura;        // +66  tamanho.x (0 = from the text)
};

} // namespace api
