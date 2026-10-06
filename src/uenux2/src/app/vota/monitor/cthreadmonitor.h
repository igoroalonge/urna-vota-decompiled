// uenux2/src/app/vota/monitor/cthreadmonitor.h   (path inferred from cthreadmonitor.cpp, attested by
// std::source_location records at lines 52, 93, 103, 117, 127, 153, 156, 165, 167, 225, 258, 260)
// Reconstructed from vota_web_wasm.wasm (unit u26).
//
// vota::CThreadMonitor is the third thread of the VOTA application (besides CThreadEleitor = voter terminal
// and CThreadOperador = mesário terminal). Every 500 ms it watches the hardware:
//   * power supply and battery charge (comum::util::CMonitoraAlimentacao, logged only on changes);
//   * voter headphones plugged / unplugged ("Fone de ouvido conectado/desconectado");
//   * the power key: "Mudança do estado da chave: URNA DESLIGADA" -> stop the other threads, SOS beep;
//   * the external flash (memory card / MV) and the voter keyboard: a missing device throws a fatal
//     error (CUeVotaError 9390 / 9391) that ends in the "URNA ELETRÔNICA INOPERANTE" screen;
// and every 30 minutes logs free/used space of both flash memories, RAM usage and the power status.
//
// RTTI: api::CThread <- vota::CThreadVota <- vota::CThreadMonitor (typeinfo @1600968, vtable @1600768):
//   [0] ~CThreadVota (func 2126; CThreadMonitor adds no destructor of its own)
//   [1] deleting destructor (func 10227)
//   [2] Run (func 10226)
//   [3] CThreadVota::TrataExcecao(const CError&) (7710)   [4] CThreadVota::TrataExcecaoDesconhecida (7709)
//   [5] FinalizaExecucao (func 10225)
//
// Web build: never started. main() registers vota::CExecucaoVotaCooperativa as IExecucaoVota; only the urna
// policy vota::CExecucaoVota (funcs 10232..10236) creates/starts this thread. The loop would abort anyway:
// its sleep_for(500ms) is compiled to emscripten_sleep(500) and the build has no Asyncify.
#pragma once

#include <cstdint>
#include <ctime>
#include <memory>
#include <mutex>

#include "vota/comum/cthreadvota.h"      // vota::CThreadVota (path inferred)

namespace vota {

class CThreadMonitor : public CThreadVota {
public:
    /// wasm func 1898 (formerly shown by the tools as "comum::util::CMonitoraAlimentacao::CreateInst", after the
    /// inlined srcloc).
    /// Lazy singleton: std::unique_ptr @1911596, mutex @1911572 (unlock residue only).
    /// Callers: vota::CExecucaoVota slots 2/3/4/6 and vota::CThreadOperador::FinalizaExecucao (10203).  name inferred
    static CThreadMonitor& GetInst();

    /// cthreadmonitor.cpp:52 - inlined into func 1898.
    CThreadMonitor();

    // ~CThreadMonitor(): implicit; the complete destructor is CThreadVota's (func 2126), the deleting
    // one is func 10227.

protected:
    void Run() override;                        // slot 2, func 10226
    void FinalizaExecucao() override;           // slot 5, func 10225

private:
    // All inlined into Run (func 10226); names from the std::source_location records.
    void MonitorFoneOuvido();                   // :127
    bool VotacaoSuspensa();                     // :93
    void SaiPorVotacaoSuspensa();               // :103 is its std::async lambda (func 10224, unit u19)
    void MostraMensagemDesligamento();          // :117 - record present, no referencing code left   ?
    void VerificaErroCriticoFlash();            // :153 / :156
    void VerificaErroCriticoKbdTE();            // :165 / :167
    void LogaEspaco() const;                    // :225
    void LogaMemoria() const;                   // name inferred (no srcloc: no singleton lookup inside)
    void LogaStatusRedeAcBateria() const;       // :258 / :260

    static constexpr std::time_t INTERVALO_LOG_S = 1800;   // 30 minutes between disk/memory/power logs

    // ---- layout (48 bytes, operator new(48) in func 1898) ------------------------------------------
    // api::CThread     +0 vptr, +4 m_estado, +8 m_bParar, +9 m_bDormindo, +12 m_pImpl, +16 m_pSync
    // vota::CThreadVota +20 tick map (begin +20, root +24, size +28), +32 unique_ptr<CAppStateContext>
    bool m_bFoneConectado;                      // +36 last headphone state (true = connected)
    std::int64_t m_proximoLog;                  // +40 time_t of the next disk/memory/power log

    static std::mutex s_mutex;                               // @1911572
    static std::unique_ptr<CThreadMonitor> s_instancia;      // @1911596, reset at exit by func 10229
};

} // namespace vota
