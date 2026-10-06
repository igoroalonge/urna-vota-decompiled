// uenux2/src/app/vota/comum/cassinadorvota.h   (path inferred: RTTI vota::CAssinadorVota; the header is included
// under this name by vota/comum/csincronizavota.cpp, unit u07)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// vota::CAssinadorVota = comum::CAssinador bound to the VOTA application (ESavdAplic 1). CAssinador asks the SAVD
// ("subsistema de assinatura e verificação digital", the urna's signing service; simulated by CWasmSavd in the web
// build) to sign files of the dinamico/ tree or to build the signature package of the result files.
//
// RTTI: vota::CAssinadorVota : comum::CAssinador, vtable @1532312 = {7766 ~CAssinadorVota (thunk to 1007),
//       4694 deleting dtor (shared with CAssinador)}. No method of its own besides the constructor.
#pragma once

#include "comum/gravadores/cassinador.h"   // comum::CAssinador (unit u23), ESavdPacote, ESavdAplic

namespace vota {

class CAssinadorVota : public comum::CAssinador
{
public:
    explicit CAssinadorVota(comum::ESavdPacote pacote);   // wasm func 1501 (CAssinador's constructor inlined)
    ~CAssinadorVota() override = default;                  // wasm func 7766
};

} // namespace vota
