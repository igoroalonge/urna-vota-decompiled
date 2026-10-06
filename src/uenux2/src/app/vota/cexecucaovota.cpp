// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/cexecucaovota.cpp
// Executa/Inicia/Aguarda (10236/10235/10234): src/uenux2/src/app/vota/u18-foreign-fragments.cpp.
//
// WEB BUILD: never instantiated (main() registers CExecucaoVotaCooperativa before anyone asks
// IExecucaoVota::GetInst()). On the urna, CThread::Start creates a pthread; in this build the thread
// factory (simulador::CWasmThread) fails with "thread constructor failed: Not supported" (u10 §6.1).
#include "vota/cexecucaovota.h"

#include "vota/monitor/cthreadmonitor.h"
#include "vota/operador/cthreadoperador.h"

namespace vota {

// wasm func 10233 - slot 5. Starts only the operator thread (the voter thread is the caller).
void CExecucaoVota::IniciaOperador()
{
    CThreadOperador::GetInst().Start();                                  // func 270 -> api::CThread::Start (1684)
}

// wasm func 10232 - slot 6. Asks the operator and monitor threads to leave their Run() loops: sets the
// api::CThread "stop" flag (+8) of both. The voter thread is not touched (it is the one calling this;
// the symmetric CThreadOperador::FinalizaExecucao, func 10203, stops the voter and monitor threads).
void CExecucaoVota::FinalizaThreads()
{
    CThreadOperador::GetInst().m_bParar = true;                          // +8
    CThreadMonitor::GetInst().m_bParar = true;                           // func 1898 (creates the CThreadMonitor if needed)
}

}  // namespace vota
