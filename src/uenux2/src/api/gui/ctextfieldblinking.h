// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfieldblinking.h
// (path inferred from ctextfieldblinking.cpp, srcloc records :39, :42, :70).
//
// api::CTextFieldBlinking : api::IFormField<api::IScreen>   (typeinfo 1582544, vtable @1582456, 68 bytes)
// Text that alternates between two colours every 500 ms: "VOTO NULO", "VOTO EM BRANCO", "VOTO DE LEGENDA",
// "NÃO HÁ CANDIDATOS CONCORRENDO" on the voter screens (colours 2 black / 3 grey on 1 white).
// Vtable: 0 dtor (10942)  1 deleting (10941)  2 Draw (10946)  3 Start (10945)  4 Stop (10944)
//         7 GetClassName (10940)  8 Rect (10943)  9 Move (shared body 2241)
#pragma once

#include <memory>
#include <string>

#include "api/gui/iformfield.h"
#include "api/gui/ctextsource.h"
#include "api/util/itimer.h"   // api::ITimer, api::ITimerScheduler (other unit)

namespace api {

class CTextFieldBlinking : public IFormField<IScreen> {
public:
    // srcloc :39/:42. Inlined into CFormBuilder::AddBlinkingText (wasm func 1265), which passes
    // (pos, make_shared<CFixedText>(alinhamento, texto), fonte, 2, 3, 1).
    CTextFieldBlinking(const SPoint& pos, const SharedIText& texto, const SFont& fonte, TColor cor1,
                       TColor cor2, TColor corFundo);
    ~CTextFieldBlinking() override;                                      // wasm func 10942 / 10941

    void Draw(IScreen& tela) const override;                             // wasm func 10946
    void Start() override;                                               // wasm func 10945
    void Stop() override;                                                // wasm func 10944
    std::string GetClassName() const override;                           // wasm func 10940
    SRect Rect() const override;                                         // wasm func 10943 (:70)
    void Move(const SPoint& pos) override;                               // shared body 2241

private:
    SPoint                  m_pos;         // +24
    SharedIText             m_texto;       // +28/+32
    SFont                   m_fonte;       // +36
    TColor                  m_cor1;        // +44 colour while m_fase is true
    TColor                  m_cor2;        // +48 colour while m_fase is false
    TColor                  m_corFundo;    // +52
    std::shared_ptr<ITimer> m_timer;       // +56/+60 500 ms, toggles m_fase
    bool                    m_fase = true; // +64
};

} // namespace api
