// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/operador/outrasopcoes/ccontadoresbiometria.cpp  (path inferred)
// Class declaration: ../estadosoperador.u34.h. Constructor inlined into CEscolheOpcao::ProcessInput (10689):
//   CAppState(2); lines 1-3 right-aligned at column 35: "Habilitação biométrica: NNNN", "Habilitação
//   biográfica: NNNN", "Habilitação sem biometria: NNNN" (or "n.a."; data sources in cescolheopcao.u02.cpp);
//   (1,4) "CORRIGE: retornar"; control input.
// Offered only on a biometric urna outside voter-training mode, so never in the public simulator.
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/estadosoperador.u34.h"
#include "vota/operador/outrasopcoes/cescolheopcao.h"

namespace vota {

// wasm func 10698 - vtable slot 7 (ProcessInput). srcloc cinteractiveform.h:57 @1587980.
void CContadoresBiometria::ProcessInput()
{
    if (m_form->Read() == api::EInputResult::CORRIGE)                   // 5
        m_proximoEstado = &CEscolheOpcao::GetInst();                    // func 2753: back to the menu
}

}  // namespace vota
