// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cpreshowclearscreen.h (path inferred).
//
// api::CPreShowClearScreen : api::IPreShow<api::IScreen>   (typeinfo 1577908, vtable @1577896, 4 bytes)
// Pre-show hook that clears the voter display to the background colour. Used by
// comum::CRegistrarMesarios (func 11140 -> 6117: "Registrar mesários?" shows a blank voter screen).
// Vtable: 0 ICF 174  1 ICF 144  2 PreShow (11139).
#pragma once

#include "api/gui/iformfield.h"   // api::IPreShow

namespace api {

class CPreShowClearScreen : public IPreShow<IScreen> {
public:
    void PreShow(IScreen& tela) override;                            // wasm 11139
};

} // namespace api
