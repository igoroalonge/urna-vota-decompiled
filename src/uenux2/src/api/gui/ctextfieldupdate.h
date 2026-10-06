// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfieldupdate.h
// (path inferred from ctextfieldupdate.cpp, srcloc records :38, :73).
//
// api::CTextFieldUpdate : api::IFormField<api::IScreen>   (typeinfo 1583068, vtable @1582996, 80 bytes)
// A text field that polls its text source with a timer and asks for a redraw only when the text changed.
// Used by CFormBuilder::AddStatusHeader (clock / battery / status line of the screens), the keyboard test
// (200 ms) and api_f3058 / api_f5936.
// Vtable: 0 dtor (5495)  1 deleting (10917)  2 Draw (10916)  3 Start (10915)  4 Stop (10914)
//         7 GetClassName (10912)  8 Rect (10913)  9 Move (shared body 2241)
#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "api/gui/iformfield.h"
#include "api/gui/ctextsource.h"
#include "api/util/itimer.h"

namespace api {

class CTextFieldUpdate : public IFormField<IScreen> {
public:
    // srcloc :38. The two colours are constant-propagated (every caller passes 2 = black on 1 = white),
    // so the compiled constructor only takes (pos, texto, periodo, fonte).
    CTextFieldUpdate(const SPoint& pos, const SharedIText& texto, const std::chrono::milliseconds& periodo,
                     const SFont& fonte, TColor corTexto = TColor{2}, TColor corFundo = TColor{1});   // func 3658
    ~CTextFieldUpdate() override;                                        // wasm func 5495 / 10917

    void Draw(IScreen& tela) const override;                             // wasm func 10916
    void Start() override;                                               // wasm func 10915
    void Stop() override;                                                // wasm func 10914
    std::string GetClassName() const override;                           // wasm func 10912
    SRect Rect() const override;                                         // wasm func 10913 (:73)
    void Move(const SPoint& pos) override;                               // shared body 2241

private:
    SPoint                  m_pos;              // +24
    SharedIText             m_texto;            // +28/+32
    SFont                   m_fonte;            // +36
    TColor                  m_corTexto;         // +44
    TColor                  m_corFundo;         // +48
    mutable std::string     m_ultimoTexto;      // +52 text painted by the last Draw
    mutable SRect           m_rectAnterior{};   // +64
    std::shared_ptr<ITimer> m_timer;            // +72/+76
};

} // namespace api
