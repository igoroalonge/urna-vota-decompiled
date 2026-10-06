// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cinteractiveformbuilder.cpp (attested by srclocs :76/:86; see
// cinteractiveformbuilder.u15.cpp for CInputFieldControlBase<IScreen>(teclas), func 5565, and
// cformbuilder.u02.cpp for the other AddControlInput overload, func 901).
#include <memory>

#include "api/gui/cformbuilder.h"
#include "api/gui/cinputfieldcontrol.h"
#include "api/gui/cinteractiveformbuilder.h"

namespace api {

// wasm func 5530 (tools: api_f5530)                                                    name inferred
// Adds an invisible input field (CInputFieldControl<IScreen>, maximum length 0) that finishes the form's
// Read() on the control keys selected by `teclas` (bit 0 BRANCO, bit 1 CORRIGE, bit 2 CONFIRMA).
// std::make_shared: one 76-byte block (12-byte control block + 64-byte field); the base constructor is func
// 5565, then the vptr of CInputFieldControl<IScreen> (@1577340) is stored; CFormBuilder::Add (func 426) names it
// "CInputFieldControl<n>".
// Callers: vota::CMostraQRCodeBU::StartState (12055, "BU digital" on the voter screen) and the voter
// start-up routine (7787, screens of CTelasVota).
std::shared_ptr<CInputFieldControl<IScreen>> CInteractiveFormBuilder::AddControlInput(CFormBuilder& builder,
                                                                                     unsigned teclas)
{
    auto campo = std::make_shared<CInputFieldControl<IScreen>>(teclas);
    builder.Add(campo);                                                  // func 426
    return campo;
}

}  // namespace api
