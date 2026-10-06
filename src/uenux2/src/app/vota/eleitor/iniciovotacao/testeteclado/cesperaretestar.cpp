// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp
// ProcessTickNaoDesligamento (11823) and the countdown text source (11826): cesperaretestar.u26.cpp.
// GetInst (5936): ctesteteclado.u02.cpp.
//
// WEB BUILD: unreachable (no keypad test in the simulator's start-up).
#include "vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.h"

#include <chrono>

#include "api/util/cdatetime.h"
#include "vota/eleitor/cthreadeleitor.h"

namespace vota::testeteclado {

/// @1835000 (12 bytes): the moment from which the test may be repeated (u26: g_horaRetestar).
extern api::CDateTime g_horaRetestar;

// wasm func 11825 - vtable slot 2
void CEsperaRetestar::StartState()
{
    m_segundosEspera += 5;                                           // +40
    m_proximoEstado = this;

    api::CDateTime horario;                                          // func 479: now
    horario += std::chrono::seconds(m_segundosEspera);               // func 2233 (CDateTime += seconds)
    g_horaRetestar = horario;

    CThreadEleitor::GetInst().StartTick(m_tick);                     // func 316 -> 700
    m_tela->Show();                                                  // +28
}

// wasm func 11824 - vtable slot 5
void CEsperaRetestar::FinishState()
{
    CThreadEleitor::GetInst().StopTick(m_tick);                      // func 422
    m_tela->Deactivate();   // inlined IForm::Deactivate (u17 name): m_ativo (+4) = false under the form's
                            // mutex (+28, unlock stub only), then every field's slot 4 (Stop: timers/animations)
}

}  // namespace vota::testeteclado
