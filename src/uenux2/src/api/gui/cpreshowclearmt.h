// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cpreshowclearmt.h (path inferred).
//
// api::CPreShowClearMT : api::IPreShow<api::IScreenMT>   (typeinfo 1579876, vtable @1579864, 4 bytes)
// The pre-show hook of (almost) every microterminal form: wipe the LCD before the fields are drawn.
// Created by shared_f301 (make_shared<CInteractiveForm<IScreenMT, IInputMT>>(campos, CPreShowClearMT, ...))
// and vota_f1694. Vtable: 0 ICF 174  1 ICF 144 (trivial dtors)  2 PreShow (11047).
#pragma once

#include "api/gui/iformfield.h"   // api::IPreShow

namespace api {

class CPreShowClearMT : public IPreShow<IScreenMT> {
public:
    void PreShow(IScreenMT& tela) override;                          // wasm 11047
};

} // namespace api
