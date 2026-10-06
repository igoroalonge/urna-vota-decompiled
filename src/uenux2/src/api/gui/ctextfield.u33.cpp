// uenux2/src/api/gui/ctextfield.cpp (+ ctextfieldblinking.cpp, ctextfieldupdate.cpp)  -- FRAGMENT written
// by unit u33. The three Rect() methods are reconstructed by unit u16 (ctextfield.cpp :55,
// ctextfieldblinking.cpp :70, ctextfieldupdate.cpp :73); this file documents their shared wasm body.
#include <source_location>
#include <string>

#include "api/gui/ctextfield.h"
#include "api/gui/iscreen.h"
#include "api/pattern/cpolysingleton.h"

namespace api {

// wasm func 3889 (observed executing: status header and every text of the voter screens)
// merge-similar-functions body of
//     CTextField::Rect() const          (thunk 10951, source_location ctextfield.cpp:55)
//     CTextFieldBlinking::Rect() const  (thunk 10943, ctextfieldblinking.cpp:70)
//     CTextFieldUpdate::Rect() const    (thunk 10913, ctextfieldupdate.cpp:73)
// The three classes have the same members (+24 SPoint m_pos, +28/+32 SharedIText m_texto, +36 SFont m_fonte) and the
// same code; only the std::source_location of the IScreen lookup differs, so it became the 3rd parameter
// (the result SRect is returned through the 1st, the sret pointer).
static SRect RectTextoAlinhado(const SPoint& pos, const IText& texto, const SFont& fonte,
                               const std::source_location& onde)                    // name inferred
{
    IScreen& tela = CPolySingleton<IScreen>::instance(GetPolySingletonsInfo(), onde);  // func 512
    TPosition largura = 0, altura = 0;
    tela.GetFontMetrics(largura, altura, fonte, texto.GetText());   // IScreen slot 3; IText slot 2
    TPosition x = pos.x;
    switch (texto.GetAlignment()) {                                  // IText slot 3
    case ETextAlignment::Right:  x = static_cast<TPosition>(pos.x - largura);      break;   // 1
    case ETextAlignment::Center: x = static_cast<TPosition>(pos.x - largura / 2);  break;   // 2 (x + w / -2)
    default:                                                                         break;
    }
    return SRect(SPoint{x, pos.y}, SPoint{static_cast<TPosition>(x + largura),
                                          static_cast<TPosition>(pos.y + altura)});
}

//   SRect CTextField::Rect() const         { return RectTextoAlinhado(m_pos, *m_texto, m_fonte, current()); }
//   SRect CTextFieldBlinking::Rect() const { ... }   SRect CTextFieldUpdate::Rect() const { ... }
// (in the source each Rect() has this body written out; the helper above is only how the binary shares it)

} // namespace api
