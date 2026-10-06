// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp
//
// srcloc evidence:
//   :45  StartState   api::IBeep lookup
//   :53  ProcessTick  comum::IInterfaceInit lookup (DesligarUrna)
//   :54  ProcessTick  api::IPower lookup
// Not executed in the recorded sessions (web IPower never reports "battery").

#include "vota/eleitor/iniciovotacao/cexibealertadesligamento.h"

#include <memory>

#include "api/hwil/ibeep.h"
#include "api/hwil/ipower.h"
#include "comum/iinterfaceinit.h"
#include "vota/log/clogvota.h"

namespace vota {

extern api::CDateTime g_dataHoraDesligamento;   // @1833312, written by CEstadoComDesligamentoAutomatico

// wasm func 12018 — vtable slot 2
void CExibeAlertaDesligamento::StartState()
{
    m_proximoEstado = this;
    m_tela->Exibe();                                         // form slot 2
    m_horaDesligamento = g_dataHoraDesligamento;             // +12 <- @1833312
    api::IBeep::GetInst().Tom(2000, 100);                    // :45  IBeep slot 1 = (Hz, 10 ms units):
                                                             //      CWasmBeep -> js_wasm_beep_queue(2000, 1000)
}

// wasm func 12017 — vtable slot 8
void CExibeAlertaDesligamento::ProcessTick(uebyte tick)
{
    if (tick != m_tick)
        return;

    const api::CDateTime agora;                                             // api_f479
    if (m_horaDesligamento.Compare(agora) < 0) {                            // func 759: deadline passed
        CLogVota::GetInst().Loga(2, "Tempo limite de espera usando bateria interna atingido");   // CLoga::loga level 2
        comum::IInterfaceInit::GetInst().DesligarUrna();                    // :53 (func 5894)
        return;
    }

    auto& energia = api::IPower::GetInst();                                 // :54
    energia.AtualizaStatus(energia.m_status);                               // IPower slot 15 (name as in u06)
    if ((energia.m_status & 6) == 2)                                        // still on internal battery
        return;
    m_proximoEstado = m_estadoAnterior;                                     // power is back: resume
}

// wasm func 2883 — slot 0, ~CExibeAlertaDesligamento(): releases m_tela (shared_ptr at +24/+28).
// wasm func 12019 — slot 1, deleting destructor: ~CExibeAlertaDesligamento(); operator delete(this).
// wasm func 12022 — at-exit destructor of the singleton pointer @1834016 (registered by func 5979):
//     auto* p = s_inst; s_inst = nullptr; if (p) { p->~CExibeAlertaDesligamento(); operator delete(p); }

}  // namespace vota
