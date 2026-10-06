// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/cstepsprogressbar.h
// (path inferred from cstepsprogressbar.cpp, attested by the srcloc record :428 of the constructor).
//
// api::CStepsProgressBar : api::IFormField<api::IScreen>   (typeinfo 1581932, vtable @1581876, 100 bytes)
//
// The "steps" bar drawn across the top of every voting screen (y = 2..30): one arrow-shaped segment per
// cargo (and per choice of a multi-choice cargo, e.g. "Senador - 1º") of the current election; the
// cargo being voted is black with white text, the ones already voted are grey, the next ones light grey.
// It is built on the stack and drawn by vota::CPreShowProgressBar::PreShow (wasm func 13550, unit u04),
// which inlines the constructor; the object is destroyed right after Draw().
//
// Vtable: 0 ~CStepsProgressBar (3665)  1 deleting dtor (10973)  2 Draw (5504)  3/4 base no-ops
//         5 RedrawIfDirty (4118)  6 SetForm (3037)  7 GetClassName (10969)  8 Rect (10968)  9 Move (10970)
#pragma once

#include <string>
#include <vector>

#include "api/gui/iformfield.h"   // api::IFormField<IScreen>, SRect, SPoint, SFont, TColor (other units)
#include "api/gui/iscreen.h"      // api::IScreen, api::SPathElement (path element, 56 bytes)

namespace api {

class CStepsProgressBar : public IFormField<IScreen> {
public:
    // cstepsprogressbar.cpp:428 ("Quantidade de segmentos deve ser maior que zero"). The only caller passes
    // the colours 3, 2, 36, 2, 1, 3, 0 (listed in member order below).
    CStepsProgressBar(const uedword atual, const std::vector<std::string>& rotulos, const SRect& area,
                      const TColor corConcluido, const TColor corAtual, const TColor corFuturo,
                      const TColor corTextoConcluido, const TColor corTextoAtual, const TColor corTextoFuturo,
                      const TColor corContorno);
    ~CStepsProgressBar() override;                                      // wasm func 3665 / 10973

    void Draw(IScreen& tela) const override;                            // slot 2, wasm func 5504
    std::string GetClassName() const override;                          // slot 7, wasm func 10969
    SRect Rect() const override;                                        // slot 8, wasm func 10968
    void Move(const SPoint& pos) override;                              // slot 9, wasm func 10970

    void SetAtual(uedword indice);   // inlined into the constructor only                // name inferred

    // One segment ready to be painted (28 bytes).
    struct SSegmento {
        std::vector<SPathElement> caminho;   // +0  outline (IScreen::FillPath / DrawPath)
        SPoint                    posTexto;  // +12 top-left corner of the label
        std::string               texto;     // +16 label ("" when no font size fits)
    };

    // Result of a layout attempt (28 bytes): font size, one rectangle per segment, the labels.
    struct SLayout {
        TFontSize                fonte = 10; // +0
        std::vector<SRect>       retangulos; // +4
        std::vector<std::string> rotulos;    // +16
    };

private:
    void CalculaSegmentos(IScreen& tela) const;   // inlined into Draw                  // name inferred

    // IFormFieldBase<IScreen>: +0 vptr, +4 bool m_precisaRedesenhar, +8 IForm* m_pForm, +12 std::string m_nome
    uedword                  m_atual = 0;           // +24 index of the current step (== size(): all done)
    std::vector<std::string> m_rotulos;             // +28
    SRect                    m_area;                // +40
    TColor                   m_corConcluido;        // +48  3  (#808080)  fill of the steps already done
    TColor                   m_corAtual;            // +52  2  (black)    fill of the current step
    TColor                   m_corFuturo;           // +56  36 (#c9c9c9)  fill of the next steps
    TColor                   m_corTextoConcluido;   // +60  2  (black)
    TColor                   m_corTextoAtual;       // +64  1  (white)
    TColor                   m_corTextoFuturo;      // +68  3  (#808080)
    TColor                   m_corContorno;         // +72  0  (transparent) outline, 1 px
    mutable bool             m_recalcular = true;   // +76  segments must be (re)computed on next Draw
    mutable SFont            m_fonte{10, 1};        // +80  chosen size, bold
    mutable std::vector<SSegmento> m_segmentos;     // +88
};

} // namespace api
