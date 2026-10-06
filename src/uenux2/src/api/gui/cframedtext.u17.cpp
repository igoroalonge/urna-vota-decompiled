// FRAGMENT reconstructed by unit u17 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cframedtext.cpp (owner: unit u15, see cframedtext.u15.h/.cpp).
// The tools attributed this function to primitives.cpp and named it "api::SRect::Top" because the
// SRect::Top/Bottom setters (primitives.cpp:26/:38) are inlined into it. Merge into cframedtext.cpp.
#include "api/gui/cframedtext.u15.h"

#include <string>

#include "api/gui/iscreen.h"

namespace api {

// wasm func 5537 (observed executing)                                            name from u15
// Draws character `c` centred inside box `i` of the digit-box row. The box rectangle is shrunk by the
// inter-box spacing horizontally and by the font's vertical margin, with the checked SRect setters.
// Callers: CFramedText::MaskText (2780), CGrayedFramedText::MaskText (5534).
void CFramedText::DesenhaCaracter(size_t i, char c, IScreen& tela) const
{
    const CFixedText texto(ETextAlignment::Center, std::string(1, c));     // vtable CFixedText @1532648

    SRect caixa = CaixaRect(i);                                           // inlined (see u15)
    const TPosition espacamento = Espacamento();                          // size <= 7 ? 1 : size / 8
    const TPosition margem = m_fonte.size <= 9 ? 1 : static_cast<TPosition>(m_fonte.size / 10);
    caixa.Left(static_cast<TPosition>(caixa.left + espacamento));         // func 5488 (primitives.cpp:20)
    caixa.Right(static_cast<TPosition>(caixa.right - espacamento));       // func 5487 (primitives.cpp:32)
    caixa.Top(static_cast<TPosition>(caixa.top + margem));                // inlined   (primitives.cpp:26)
    caixa.Bottom(static_cast<TPosition>(caixa.bottom - margem));          // inlined   (primitives.cpp:38)

    const SPoint centro{static_cast<TPosition>(caixa.left + (caixa.right - caixa.left) / 2),
                        static_cast<TPosition>(caixa.top + 1)};
    tela.WriteText(centro, texto, m_fonte, /*corTexto*/ 2, /*corFundo*/ 1);   // IScreen slot 19
}

}  // namespace api
