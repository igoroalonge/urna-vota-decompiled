// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.h (path inferred from the .cpp,
// attested by srcloc lines 45, 53, 54).
//
// "Exibe alerta de desligamento" = battery warning. CEstadoComDesligamentoAutomatico (unit u06) switches
// to this state when the urna has been running on its internal battery for (limite - aviso) seconds.
// It beeps, shows the warning and, once the power-off time g_dataHoraDesligamento is reached, logs
// "Tempo limite de espera usando bateria interna atingido" and turns the urna off. If external power
// comes back first, it returns to the previous state.
//
// RTTI: comum::CAppState <- vota::CExibeAlertaDesligamento (typeinfo @1543096, vtable @1543012)
//   [0] ~dtor 2883  [1] deleting 12019  [2] StartState 12018  [8] ProcessTick 12017
#pragma once

#include <memory>

#include "api/util/cdatetime.h"
#include "comum/cappstate.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

class CExibeAlertaDesligamento final : public comum::CAppState {
public:
    /// Lazy singleton, wasm func 5979 (unit u06): object @1834016; its at-exit reset is func 12022.
    static CExibeAlertaDesligamento& GetInst();

    ~CExibeAlertaDesligamento() override = default;   // wasm func 2883 (releases m_tela); deleting: 12019
    void StartState() override;                        // wasm func 12018 (srcloc 45)
    void ProcessTick(uebyte tick) override;            // wasm func 12017 (srcloc 53, 54)

    comum::CAppState* m_estadoAnterior = nullptr;      // +36 set by CEstadoComDesligamentoAutomatico

private:
    CExibeAlertaDesligamento();                        // CAppState(4 = ticks), see func 5979

    api::CDateTime m_horaDesligamento{-1};             // +12 (12 bytes)
    std::shared_ptr<api::IForm<api::IScreen>> m_tela;  // +24 (+28) CTelasVota +44/+48
    uebyte m_tick;                                     // +32 1000 ms tick of CThreadEleitor
};

}  // namespace vota
