// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cformbuildermt.h (path inferred; template member of the microterminal form
// builder api::CFormBuilderMT = std::vector<std::shared_ptr<IFormField<IScreenMT>>>).
//
// Unlike the voter-screen builder (CFormBuilder::Add, func 426, which gives every field a unique name), the MT
// builder only appends the field (vector<shared_ptr>::push_back, shared_f1400). The objects are created with
// `new` and wrapped in std::shared_ptr (control block __shared_ptr_pointer), not with make_shared.
#include <memory>

#include "api/gui/cdatatext.h"
#include "api/gui/ctextfieldmt.h"
#include "api/gui/ctextsource.h"

namespace api {

// wasm func 5409 (tools: api_f5409)                                                    name inferred
// Instantiation of CFormBuilderMT::Add<CTextFieldMT>(pos, texto) with the text source built in place:
// a CDataText<CTextSource> (16 bytes, vtable @1590656: +4 alinhamento = 0 (left, constant-propagated),
// +8 CTextSource = shared_ptr<std::string>) that re-reads the shared string at every draw.
// Callers: CMostraEleitorVotando's constructor (inlined in func 1150: line 4, the "VOTANDO PARA: <cargo>" text
// updated by the voter thread) and CEleitorDemorando::GetInst (func 2734, "o eleitor está demorando").
std::shared_ptr<IFormField<IScreenMT>> CFormBuilderMT::AddTextoCompartilhado(const SPoint& pos, CTextSource fonte)
{
    std::shared_ptr<IText> texto(new CDataText<CTextSource>(ETextAlignment(0), fonte));   // copy: use_count + 1
    std::shared_ptr<IFormField<IScreenMT>> campo(new CTextFieldMT(pos, texto));        // 48 bytes, ctor 1262
    m_campos.push_back(campo);                                                          // shared_f1400
    return campo;
}

}  // namespace api
