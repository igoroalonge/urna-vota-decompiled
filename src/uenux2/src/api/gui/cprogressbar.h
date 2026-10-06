// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/cprogressbar.h (path inferred
// from cprogressbar.cpp, attested by the srcloc records :52/:58/:67 of the constructor).
//
// api::CProgressBar : api::IFormField<api::IScreen>   (typeinfo 1581856, vtable @1581768, 88 bytes: make_shared
// allocates 100 = 12-byte control block + 88)
//
// A horizontal progress bar (barra de progresso). Used twice in VOTA:
//   * vota::CTelasVota (func 7787): the "Gravando" screen shown after the last CONFIRMA of a voter,
//     0..4 steps, rect {70,225}-{570,255}, no text (empty format);
//   * vota::CProgressoEncerramento (inside vota::CGravaResultado::vf2): "Preparando dados para
//     encerramento", 0..30 steps, rect {70,350}-{570,385}, format "%p" (percentage).
// Both go through CFormBuilder::AddProgressBar (wasm func 5545, see cformbuilder.u16.cpp), which inlines
// the constructor below.
//
// Vtable (IFormFieldBase<IScreen> slots, see ctextfield.h for the interface recap):
//   0 ~CProgressBar() (5509)   1 deleting dtor (10978)   2 Draw (10977)   3/4 Start/Stop = base no-ops
//   5 RedrawIfDirty (base 4118)   6 SetForm (base 3037)   7 GetClassName (10975)   8 Rect (10974)   9 Move (10976)
#pragma once

#include <string>

#include "api/gui/iformfield.h"   // api::IFormField<IScreen>, SRect, SPoint, TColor, TFontSize (other units)

namespace api {

class CProgressBar : public IFormField<IScreen> {
public:
    // cprogressbar.cpp:52/58/67. Parameter order of the three uedwords is inferred: every caller passes
    // (0, 0, maximo), so only "the third one is the upper limit" is certain.            // ? order
    CProgressBar(const uedword valorInicial, const uedword minimo, const uedword maximo, const SRect& area,
                 TColor corBarra, TColor corFundo, TColor corTextoPreenchido, TColor corTexto,
                 const std::string& formato, TFontSize tamanhoFonte, uebyte margem, const TColor corBorda);
    ~CProgressBar() override;                                           // wasm func 5509 / 10978

    void Draw(IScreen& tela) const override;                            // slot 2, wasm func 10977
    std::string GetClassName() const override;                          // slot 7, wasm func 10975
    SRect Rect() const override;                                        // slot 8, wasm func 10974
    void Move(const SPoint& pos) override;                              // slot 9, wasm func 10976

    void SetValor(uedword valor);   // wasm func 3667 (attributed to unit u37)          // name inferred
    void Incrementa();              // wasm func 5508 (attributed to unit u07)          // name inferred

private:
    // IFormFieldBase<IScreen>: +0 vptr, +4 bool m_precisaRedesenhar, +8 IForm* m_pForm, +12 std::string m_nome
    uedword     m_valor = 0;            // +24
    uedword     m_minimo;               // +28
    uedword     m_maximo;               // +32
    SRect       m_area;                 // +36  outer rectangle (border)
    SRect       m_areaInterna;          // +44  m_area.Adjusted(1, 1, -3, -3): where the bar is filled
    TColor      m_corBarra;             // +52  13 (#006400, dark green) for both callers
    TColor      m_corFundo;             // +56  1 (white)
    TColor      m_corTextoPreenchido;   // +60  1 (white): text over the filled part
    TColor      m_corTexto;             // +64  2 (black): text over the empty part
    std::string m_formato;              // +68  "%p" -> percentage, "%v" -> value; "" = no text
    TFontSize   m_tamanhoFonte;         // +80  0 = automatic (80 % of the inner height)
    uebyte      m_margem;               // +82  2: border width
    TColor      m_corBorda;             // +84  2 (black)
};

} // namespace api
