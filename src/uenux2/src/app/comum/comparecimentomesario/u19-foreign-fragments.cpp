// FRAGMENT reconstructed by unit u19 from vota_web_wasm.wasm.
// Registration of mesários' attendance ("comparecimento de mesários"): the shared ProcessInput body of two
// MT states. Original files (attested by their srclocs, passed as parameters to the merged body):
//   uenux2/src/app/comum/comparecimentomesario/estados/cmesarioregistrado.cpp   (:72 GetControlador)
//   uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp   (:68 GetControlador)
// The callers (funcs 10338 / 10391, vtable slot 7 of the two classes) are two-line thunks
//   comum_f6011(this, &srcloc GetControlador, &srcloc cinteractiveform.h:57)
// that the tools named "...::GetControlador@N" after the first srcloc they pass.
#include "api/pattern/cpolysingleton.h"
#include "comum/comparecimentomesario/icontroladorregistramesarios.h"

namespace comum {

// wasm func 6011: merged body (wasm-opt merge-similar-functions) of
//   CMesarioRegistrado::ProcessInput() and CRegistrarMesarios::ProcessInput().
// GetControlador() = CPolySingleton<IControladorRegistraMesarios>::instance(info, loc) (func 356).
void CMesarioRegistrado::ProcessInput()   // (and CRegistrarMesarios::ProcessInput, same code)
{
    switch (m_form->Read()) {                            // (+12, pointer) cinteractiveform.h:57 -> CPolySingleton<IInputMT> (383)
    case 5:                                              // CORRIGE
        GetControlador().vf25();                         // slot 25 (name unknown)
        m_proximoEstado = &ProximoEstadoCorrige();       // comum_f5389: the "Finalizar registro de mesários?"
                                                         // screen ("CORRIGE: Voltar  CONFIRMA: Finalizar")
        break;
    case 9:                                              // CONFIRMA
        GetControlador().vf24();                         // slot 24 (name unknown)
        m_proximoEstado = &ProximoEstadoConfirma();      // comum_f2729 (state getter)
        break;
    default:
        break;
    }
}

}  // namespace comum
