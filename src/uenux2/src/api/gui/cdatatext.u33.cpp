// uenux2/src/api/gui/cdatatext.h + iformfield.h  -- FRAGMENT written by unit u33.
// (cdatatext.h is reconstructed by unit u32, iformfield.h by unit u32 as well.)
//
// Two destructor bodies that wasm-opt's merge-similar-functions shared between unrelated class templates.
// Both classes have the same shape - vptr at +0, two trivial words, a std::string at +12 and nothing else to
// destroy - so the compiled destructors differ only in the vtable constant, which became a parameter:
//
//   api::CDataTextFmt<SRC>   {+0 vptr, +4 ETextAlignment, +8 SRC (4-byte function pointer / small functor),
//                             +12 std::string m_formato}
//   api::IFormFieldBase<MEDIA> {+0 vptr, +4 bool m_precisaRedesenhar, +8 IForm* m_form, +12 std::string m_nome}
#include <string>

#include "api/gui/cdatatext.h"
#include "api/gui/iformfield.h"

namespace api {

// wasm func 1566 - "complete-object destructor" body (D1): stores the vtable passed as 2nd parameter, frees
// the string at +12 if it is long, returns this. 12-byte thunks call it with their own vtable:
//   IFormFieldBase<IScreen>::~IFormFieldBase    12658      IFormFieldBase<IScreenMT>  11052
//   IFormFieldBase<IPaper>                       11013
//   CDataTextFmt<comum::(anon)::CComparecimentoMesariosDS>     10363, 10370, 10377
//   CDataTextFmt<std::string (*)(const std::string&)>         12323
//   CDataTextFmt<const std::string (*)(const std::string&)>   12567
template <class MEDIA> IFormFieldBase<MEDIA>::~IFormFieldBase() = default;
template <class SRC>   CDataTextFmt<SRC>::~CDataTextFmt() = default;

// wasm func 1565 - "deleting destructor" body (D0): the same, then operator delete(this) (free).
// Callers:
//   * the D0 thunks of CDataTextFmt<CComparecimentoMesariosDS> (10362, 10369, 10374),
//     CDataTextFmt<std::string (*)(const std::string&)> (12317), CDataTextFmt<const std::string (*)(...)> (12564);
//   * three ICF'd D0 bodies of IFormField leaf classes that add no member needing destruction. Their own
//     vptr store is dead (the base destructor overwrites it), so what is left is "store the IFormFieldBase<MEDIA>
//     vtable, free m_nome, free this":
//       2777 (vtable IFormFieldBase<IScreenMT>): CBeepFieldMT, CLedFieldMT, CBuzzFieldMT, CClockFieldMT  (slot 1)
//       3681 (vtable IFormFieldBase<IScreen>):   CFillField, CLineField, CRectField                   (slot 1)
//       5516 (vtable IFormFieldBase<IPaper>):    CCutFieldPaper, CNewLineFieldPaper                   (slot 1)
// (IFormFieldBase<MEDIA> itself is abstract: its own D0 slot is ICF 325, `unreachable`.)
//
//   void operator delete(void*) after ~CDataTextFmt<SRC>() / ~CLineField() ...  - implicit, no source line.

} // namespace api
