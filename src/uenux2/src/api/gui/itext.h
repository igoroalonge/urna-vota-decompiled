// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/itext.h (path inferred).
//
// api::IText (typeinfo 1530076, "class": no base) is the text given to every text field (CTextField,
// CTextFieldMT, CTextFieldPaper, CTextFieldUpdate(MT), CMaskedTextField ...). A field never stores a
// std::string: it stores a shared_ptr<IText> and asks for the current text each time it draws, which is how
// screens show live data (the number being typed, the clock, the name of the current cargo...).
//
// Implementations (all in this unit): CFixedText (constant text), CDataText<SRC> (text produced by a data
// source SRC at draw time) and CDataTextFmt<SRC> (data source + format string). See cdatatext.h.
//
// Vtable (4 slots, same in every implementation):
//   [0] ~IText   [1] deleting dtor   [2] std::string GetText() const   [3] ETextAlignment GetAlignment() const
// Slot 3 is the ICF body 1661 (`return this->+4`) in every implementation.
#pragma once

#include <memory>
#include <string>

#include "api/gui/primitives.h"   // ETextAlignment

namespace api {

class IText {
public:
    explicit IText(ETextAlignment alinhamento = ETextAlignment::Left) : m_alinhamento(alinhamento) {}
    virtual ~IText() = default;

    virtual std::string GetText() const = 0;                                        // slot 2
    virtual ETextAlignment GetAlignment() const { return m_alinhamento; }           // slot 3 (func 1661)

protected:
    ETextAlignment m_alinhamento;    // +4
};

using SharedIText = std::shared_ptr<IText>;   // attested in srcloc signatures (e.g. CTextFieldPaper ctor)

} // namespace api
