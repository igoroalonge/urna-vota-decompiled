// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaocapturada.cpp  (path inferred)
// Class declaration: ../estadosoperador.u34.h. Constructor inlined into CRegistraDigitalOperador::ProcessTick
// (func 10451, cregistradigitaloperador.cpp):
//     CAppState(2); LED off; Beep(1); (20,2) centred "Digital não capturada";
//     (40,4) right "CONFIRMA: tentar novamente"; control input.
//
// Context: a voter whose fingerprint was not recognised can be released with the MESÁRIO's own fingerprint
// (CRegistraDigitalOperador). If no finger is read within 15 s and attempts remain, this screen is shown.
//
// WEB BUILD: dead code (operator thread not run; no fingerprint reader is registered in the simulator).
#include "vota/operador/confirmaidentidade/cregistradigitaloperador.h"
#include "vota/operador/estadosoperador.u34.h"

namespace vota {

// wasm func 10458 - vtable slot 7 (ProcessInput). srcloc cinteractiveform.h:57 @1592408.
void CDigitalNaoCapturada::ProcessInput()
{
    if (m_form->Read() == api::EInputResult::CONFIRMA)
        m_proximoEstado = &CRegistraDigitalOperador::GetInst();    // func 5396: capture the mesário's finger again
}

}  // namespace vota
