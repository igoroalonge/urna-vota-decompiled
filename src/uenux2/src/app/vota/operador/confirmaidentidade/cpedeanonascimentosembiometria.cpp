// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/operador/confirmaidentidade/cpedeanonascimentosembiometria.cpp
// Constructor: operador/u10-foreign-fragments.cpp. ProcessInput (10489): operador/u27-foreign-fragments.cpp.
#include "vota/operador/confirmaidentidade/cpedeanonascimentosembiometria.h"

namespace vota {

// wasm func 10490 - vtable slot 2. No log record: the operator harness run (u10 §2) goes
// CNomeEleitor -> this state -> CInformaEleitorPodeVotar.
void CPedeAnoNascimentoSemBiometria::StartState()
{
    m_proximoEstado = this;
    m_pedindoAno = true;           // +32
    m_erros = 0;                   // +28
    m_formPedeAno->Show();         // +12
}

}  // namespace vota
