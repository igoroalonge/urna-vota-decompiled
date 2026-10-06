// FRAGMENT reconstructed by unit u16 from vota_web_wasm.wasm. Merge into uenux2/src/api/gui/cformbuilder.cpp
// (see also cformbuilder.u02.cpp / cformbuilder.u07.cpp).
//
// Three CFormBuilder helpers that the tools named after the constructor they inline (the srcloc records
// found in their bodies belong to that constructor). They are not constructors: each takes the builder,
// returns a shared_ptr through an sret pointer, make_shared's the field and appends it with
// CFormBuilder::Add (wasm func 426). Their shape matches the attested siblings AddText (202), AddRect (3680)
// and AddFill (2245) reconstructed by unit u02. All names are inferred.
#include "api/gui/cformbuilder.h"

#include "api/gui/cprogressbar.h"
#include "api/gui/ctextfieldblinking.h"
#include "api/gui/ctextrectfield.h"

namespace api {

// wasm func 1265 (observed executing). Inlines CTextFieldBlinking::CTextFieldBlinking
// (ctextfieldblinking.cpp:39/:42). Callers: CTelasVota screen builders ("VOTO NULO", "VOTO EM BRANCO",
// "VOTO DE LEGENDA", "NÃO HÁ CANDIDATOS CONCORRENDO") and api_f1190.
std::shared_ptr<CTextFieldBlinking> CFormBuilder::AddBlinkingText(const std::string& texto, const SPoint& pos,
                                                                  const SFont& fonte, ETextAlignment alinhamento)
{
    auto dado = std::make_shared<CFixedText>(alinhamento, texto);
    auto campo = std::make_shared<CTextFieldBlinking>(pos, dado, fonte, TColor{2}, TColor{3}, TColor{1});
    Add(campo);                                                                          // func 426
    return campo;
}

// wasm func 5545 (observed executing). Inlines CProgressBar::CProgressBar (cprogressbar.cpp:52/58/67).
// Callers: vota::CTelasVota constructor (func 7787: maximo 4, {70,225}-{570,255}, formato "") and
// vota::CProgressoEncerramento inside vota::CGravaResultado::vf2 (maximo 30, {70,350}-{570,385}, "%p").
std::shared_ptr<CProgressBar> CFormBuilder::AddProgressBar(uedword maximo, const SPoint& a, const SPoint& b,
                                                           const std::string& formato)
{
    auto campo = std::make_shared<CProgressBar>(0u, 0u, maximo, SRect(a, b),   // SRect normalises the corners
                                                TColor{13}, TColor{1}, TColor{1}, TColor{2}, formato,
                                                TFontSize{0} /*automatic*/, uebyte{2}, TColor{2});
    Add(campo);
    return campo;
}

// wasm func 5546 (observed executing). Inlines CTextRectField::CTextRectField (ctextrectfield.cpp:51).
// Only caller: vota::CTelasVota constructor (func 7787), for "Número de cópias" / "acima do limite
// permitido" on {0,160}-{640,195} and {0,210}-{640,245}.
std::shared_ptr<CTextRectField> CFormBuilder::AddTextRect(const std::string& texto, const SRect& area)
{
    auto dado = std::make_shared<CFixedText>(ETextAlignment::Center, texto);
    auto campo = std::make_shared<CTextRectField>(area, TColor{20} /*#ffd300*/, dado, SFont{35, 0} /*@475016*/,
                                                  TColor{2});
    Add(campo);
    return campo;
}

} // namespace api
