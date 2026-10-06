// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/ctextfieldupdatemt.h (path inferred from its screen sibling
// api::CTextFieldUpdate, ctextfieldupdate.cpp, attested by srcloc).
//
// api::CTextFieldUpdateMT : api::IFormField<api::IScreenMT>   (typeinfo 1590516, vtable @1590484, 56 bytes)
// A line of the poll worker's microterminal (4 x 40 LCD) that re-reads its text periodically and redraws
// itself only when the text changed. Used for the status line of vota::CPedeIdentidade (identification of
// the voter by the mesário: "Aguardando ...", biometric progress, 300 ms period).
//
// Vtable: 0 dtor (5411)  1 deleting (10549)  2 Draw (10548)  3 Start (10546)  4 Stop (10545)
//         5 RedrawIfDirty (4118)  6 SetForm (3037)  7 GetClassName (11051: "IFormField<IScreenMT>")
#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "api/gui/iformfield.h"
#include "api/gui/itext.h"
#include "api/util/ctimerscheduler.h"   // api::ITimer, api::ITimerScheduler

namespace api {

class CTextFieldUpdateMT : public IFormField<IScreenMT> {
public:
    // Constructor: only inlined (in vota::CPedeIdentidade::GetInst, func 652).
    CTextFieldUpdateMT(const SPoint& pos, const SharedIText& texto, const std::chrono::milliseconds& periodo);
    ~CTextFieldUpdateMT() override;                                  // wasm 5411 / 10549

    void Draw(IScreenMT& tela) const override;                       // wasm 10548
    void Start() override;                                           // wasm 10546
    void Stop() override;                                            // wasm 10545

private:
    SPoint                  m_pos;              // +24 (column, line)
    SharedIText             m_texto;            // +28/+32
    mutable std::string     m_textoAnterior;    // +36 text drawn last time
    std::shared_ptr<ITimer> m_timer;            // +48/+52
};

} // namespace api
