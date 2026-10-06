// FRAGMENT reconstructed by unit u32 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/iinputfield.h (path inferred; Read() is in iinputfield.u17.h, the
// constructors in cformbuilder.u02.cpp / cinputfieldmt.h). Merge into iinputfield.h.
#pragma once

#include "api/gui/iinputfield.u17.h"

namespace api {

// wasm func 6312 - one body for both instantiations (identical code, merged by wasm-opt):
//   slot 12 of IInputField<IScreen>, CInputFieldControl(Base)<IScreen>
//   slot 10 of IInputField<IScreenMT>, CInputFieldControl(Base)<IScreenMT>, CInputFieldMT
// (CInputField<MASK> overrides it with the version at cinputfield.h:148, func 12387, which also resizes the
// digit boxes; that srcloc's signature `virtual void api::CInputField<api::CFramedText>::SetLength(size_t)`
// attests the slot name.) Changing the maximum length discards what was typed and asks the form for a redraw.
template <class MEDIA>
void IInputField<MEDIA>::SetLength(std::size_t tamanho)
{
    if (tamanho == m_tamanhoMaximo)
        return;
    m_texto.clear();
    m_tamanhoMaximo = tamanho;
    this->Invalidate();              // m_form active ? (dirty = true, form->Redraw()) : nothing
}

} // namespace api
