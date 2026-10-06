// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cformpart.cpp (path inferred).
#include "api/gui/cformpart.h"

namespace api {

// wasm func 10888 (slot 0, unit u24: thunk into comum_f2899) / wasm func 10887 (slot 1, this unit: thunk
// into api_f6026 = release m_form + operator delete).
CFormPart::~CFormPart() = default;

// wasm func 10889 (slot 2)
// "Showing" a paper form prints it: IForm<IPaper>::Show (slot 2) puts it on the paper form stack and
// OnActivate draws every field on the IPaper device (IPaperRelatorios; simulador::CWasmNullPaper in the web).
void CFormPart::Imprime() const
{
    m_form->Show();                                                   // IForm slot 2
}

} // namespace api
