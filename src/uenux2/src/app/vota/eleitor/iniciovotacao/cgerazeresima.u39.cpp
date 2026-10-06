// FRAGMENT reconstructed by unit u39 from vota_web_wasm.wasm.
// Original file (path inferred by u07; u09 keeps StartState in cgeradorresumozeresima.cpp):
// uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.cpp
// Class declaration: cgerazeresima.h (u07). Constructor 5965: cgerazeresima.u07.cpp. StartState 11946 (the
// zerésima report itself): unit u09.
//
// RTTI: comum::CAppState <- vota::CGeraZeresimaBase (typeinfo @1544436, vtable @1544392, 20 bytes)
//   [0] dtor 1720  [1] ICF 325  [2] StartState 11946  [9] pure  [10] pure (GetEstadoResumo)
//   <- CGeraZeresima (@1544584), CRegerarZeresima (@1546500)
#include "vota/eleitor/iniciovotacao/cgerazeresima.h"

namespace vota {

// wasm func 1720 - vtable slot 0: thunk into the shared body func 2902(this, vtable CGeraZeresimaBase)
// = store the vptr, release m_tela (+12/+16); returns `this`. Direct callers: the ICF destructor thunks of
// the subclasses (5960, 5961), the singleton accessors that replace an old instance (5962 CGeraZeresima::GetInst,
// 11838 CConfirmaRegerarZeresima::ProcessInput building CRegerarZeresima) and the at-exit resets (11844, 11938).
CGeraZeresimaBase::~CGeraZeresimaBase() = default;

}  // namespace vota
