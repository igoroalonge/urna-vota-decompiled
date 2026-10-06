// Reconstructed from vota_web_wasm.wasm (unit u06).
// Original: uenux2/src/app/vota/eleitor/cestadocomdesligamentoautomatico.cpp
//
// CEstadoComDesligamentoAutomatico ("state with automatic power-off") is a base class for states
// in which the urna may sit idle for a long time (waiting for the zerésima time, application
// finished, BU / certificate QR codes on screen). Every second it checks the power supply: while
// the urna runs on its internal battery for longer than (limite - aviso) seconds it switches to
// CExibeAlertaDesligamento, which shows the shutdown warning for `aviso` more seconds.
//
// RTTI: comum::CAppState <- vota::CEstadoComDesligamentoAutomatico (typeinfo @1541948, vtable @1541908)
//         <- CVerificaHorarioZeresima, CAplicacaoEncerrada, CMostraQRCodeBU, CMostraQRCodeCertificado
//   [8] ProcessTick = func 12061 (the analyzer named it after the lambda at line 44, which is inlined)
//   [9] ProcessTickNaoDesligamento(uebyte) = hook for the subclasses' own ticks (default no-op,
//       func 425; [2] StartState is the pure one)
//
// srcloc: :44 auto vota::CEstadoComDesligamentoAutomatico::ProcessTick(uebyte)::(lambda)::operator()() const
//         (IPower lookup)

#include "comum/cappstate.h"

#include <chrono>

#include "api/hwil/ipower.h"
#include "api/util/cdatetime.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "vota/eleitor/cthreadeleitor.h"

namespace vota {

/// Moment at which the power-off happens; read by CExibeAlertaDesligamento::StartState.
api::CDateTime g_dataHoraDesligamento;                     // @1833312 (12 bytes) name inferred

class CEstadoComDesligamentoAutomatico : public comum::CAppState {
public:
    void ProcessTick(uebyte tick) override;                 // slot 8, func 12061
    virtual void ProcessTickNaoDesligamento(uebyte tick) {} // slot 9

protected:
    /// func 1285 (unit ?): CAppState(flags | 4), m_limiteBateria = CDateTime(-1) (null),
    ///                     m_tick = CThreadEleitor::GetInst().AdicionaTick(1000ms)
    explicit CEstadoComDesligamentoAutomatico(uebyte flags);

    api::CDateTime m_limiteBateria;     // +12 (date +12..+19, time +20): when the alert must show
    uebyte m_tick;                      // +24 (1 s period, running)
};

// ---------------------------------------------------------------------------------------------
// wasm func 12061 — vtable slot 8
void CEstadoComDesligamentoAutomatico::ProcessTick(uebyte tick)
{
    if (tick != m_tick) {
        ProcessTickNaoDesligamento(tick);
        return;
    }

    // lambda at line 44 (inlined): "has the battery deadline passed?"
    const api::CDateTime nulo(-1);
    if (m_limiteBateria != nulo) {
        const api::CDateTime agora;
        if (agora.Compare(m_limiteBateria) > 0) {                          // func 759
            // deadline reached: show the power-off alert for `aviso` seconds
            m_limiteBateria += std::chrono::seconds(
                comum::CConfiguracaoEleicao::GetInst().GetTempoAvisoDesligamento());   // cfg +184, func 5474
            g_dataHoraDesligamento = m_limiteBateria;
            auto& alerta = CExibeAlertaDesligamento::GetInst();            // func 5979
            alerta.m_estadoAnterior = this;                                // +36
            m_proximoEstado = &alerta;
            return;
        }
    }

    auto& energia = api::IPower::GetInst();                                // :44
    energia.AtualizaStatus(energia.m_status);                              // IPower slot 15 (name inferred)
    if ((energia.m_status & 6) == 2) {                                     // running on internal battery
        if (m_limiteBateria == nulo) {
            const auto& cfg = comum::CConfiguracaoEleicao::GetInst();
            m_limiteBateria = api::CDateTime();                            // now
            m_limiteBateria += std::chrono::seconds(
                cfg.GetTempoLimiteBateria() - cfg.GetTempoAvisoDesligamento());   // cfg +180 - cfg +184
        }
    } else {
        m_limiteBateria = api::CDateTime(-1);                              // external power: reset
    }
}

// ---------------------------------------------------------------------------------------------
// wasm func 5979 — lazy singleton of CExibeAlertaDesligamento (constructor inlined; class in
// uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp, unit u09).
//   new (40 bytes) CExibeAlertaDesligamento:
//     CAppState(4 = ticks), m_horaDesligamento = CDateTime(-1) (+12),
//     m_tela = CTelasVota::GetInst() +44/+48 (shared_ptr copy, +24/+28),
//     m_tick = CThreadEleitor::GetInst().AdicionaTick(1000ms) (+32), m_estadoAnterior = nullptr (+36)
//
// wasm func 4640 — inline helper (header): uebyte CThread::AdicionaTick(std::chrono::milliseconds p)
//                  { return m_ticks.AddTick(p.count()); }   // CTickManager at thread +20; tick starts running
// wasm func 5474 — inline helper (header): CDateTime& CDateTime::operator+=(std::chrono::seconds s)
//                  { AdicionaSegundos(s.count()); }         // func 2233: timegm + s, gmtime_r back

}  // namespace vota
