// Reconstructed from vota_web_wasm.wasm (unit u39 = constructor and slot 7; slots 2/3 by unit u18, moved here).
// Original (path inferred, see the header): uenux2/mock/app/vota/cexecucaovotacooperativa.cpp
// The database files all three functions of the class here (component app:mock): 7828, 4713, 7823.
#include "vota/cexecucaovotacooperativa.h"

#include <memory>

#include "vota/eleitor/cthreadeleitor.h"

namespace vota {

// wasm func 7828 (table slot 126; called only by main, func 10307)
CExecucaoVotaCooperativa::CExecucaoVotaCooperativa(comum::CAppState& estadoInicial)
    : m_estadoInicial(estadoInicial)
{
}

// wasm func 4713 - vtable slots 2 AND 3 (table slots 858 and 859): Executa and Inicia share this body
// (unit u18). Observed executing (votaInit calls slot 3). Installs the first voter state (+4,
// CAguardaMensagem, given by main) in a new comum::CAppStateContext of the voter thread (the old context,
// if any, is deleted through its vtable slot 1) and starts it. No thread is created.
void CExecucaoVotaCooperativa::Inicia()
{
    auto& thread = CThreadEleitor::GetInst();                                                // func 316
    thread.m_pContexto = std::make_unique<comum::CAppStateContext>(&m_estadoInicial);        // 3844, operator new(8)
    if (auto* estado = thread.m_pContexto->GetEstado())                                      // context +4
        estado->StartState();                                                                // CAppState slot 2
}

// wasm func 4713 - vtable slot 2. The same body as Inicia: either Executa only calls Inicia (inlined),
// or the two identical bodies were merged by wasm-opt.                                            // ?
void CExecucaoVotaCooperativa::Executa()
{
    Inicia();
}

// wasm func 7823 - vtable slot 7. Observed executing: 723 samples in the recorded municipal vote.
// One step of the voter state machine: CThreadEleitor::Processar (func 4349) drains the message queue,
// the keypad and the ticks and applies the CAppState transition protocol. Nothing happens when no context
// was installed yet (+32) or when the thread was asked to stop (api::CThread +8; only
// CThreadOperador::FinalizaExecucao, func 10203, sets it - never in the web build).
bool CExecucaoVotaCooperativa::Processa()
{
    auto& thread = CThreadEleitor::GetInst();                 // func 316
    if (!thread.m_pContexto)                                  // +32 std::unique_ptr<comum::CAppStateContext>
        return false;
    if (thread.m_bParar)                                      // +8
        return false;
    return thread.Processar();                                // func 4349 (repeats both tests itself)
}

}  // namespace vota
