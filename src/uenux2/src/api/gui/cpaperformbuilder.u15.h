// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cpaperformbuilder.h (srcloc cpaperformbuilder.h:51). Merge into it.
#pragma once

#include "api/pattern/cpolysingletonlist.h"

namespace api {

class IPaper;
class IPaperRelatorios;   // the report printer (in the web: CWasmNullPrinter/CWasmNullPaper)

// A form printed on paper; CLASS selects which IPaper singleton renders it.
template <class CLASS>
class IFormImpressao /* : public IForm<IPaper> */ {
public:
    // wasm func 11006 (not observed) - srcloc cpaperformbuilder.h:51, IForm slot 5 (GetRenderForm)
    IPaper& GetRenderForm() const /* override */
    {
        return CPolySingletonList::instance<CLASS>();   // func 905: instance<IPaperRelatorios>
    }
};

} // namespace api
