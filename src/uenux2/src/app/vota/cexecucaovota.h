// Reconstructed from vota_web_wasm.wasm (unit u39 = slots 5 and 6; slots 2-4 by unit u18).
// Original (path inferred, as in u18): uenux2/src/app/vota/cexecucaovota.h
// (next to iexecucaovota.cpp, attested by srcloc :74 of IExecucaoVota::GetInst, which default-registers it;
// the IExecucaoVota typeinfo @1600564 and the CExecucaoVota vtable @1600580 are adjacent in the data).
//
// vota::IExecucaoVota = how the VOTA application runs its three threads (voter CThreadEleitor, operator
// CThreadOperador, monitor CThreadMonitor). Slot protocol (names inferred by u18/u29/u30):
//   [0] dtor [1] deleting  [2] Executa (start + wait)  [3] Inicia  [4] Aguarda  [5] IniciaOperador
//   [6] FinalizaThreads  [7] bool Processa() (one cooperative step)  [8] GetFilaEleitor  [9] GetEstadoAtual
// CExecucaoVota is the URNA policy: real threads (CThread::Start = func 1684). The web build registers
// CExecucaoVotaCooperativa from main() first, so none of this is ever created in the simulator.
//
// RTTI: vota::IExecucaoVota (typeinfo @1600564) <- vota::CExecucaoVota (typeinfo @1600620, vtable @1600580,
// 4 bytes)  [0] 174 [1] 144 [2] 10236 [3] 10235 [4] 10234 [5] 10233 [6] 10232 [7] ICF 340 (return false)
// [8] ICF 4708 (&CThreadEleitor +36) [9] ICF 4705 (current voter state).
#pragma once

#include "vota/iexecucaovota.h"

namespace vota {

class CExecucaoVota final : public IExecucaoVota {
public:
    void Executa() override;                           // [2] wasm func 10236 (u18)
    void Inicia() override;                            // [3] wasm func 10235 (u18)
    void Aguarda() override;                           // [4] wasm func 10234 (u18)
    void IniciaOperador() override;                    // [5] wasm func 10233   name inferred (u18)
    void FinalizaThreads() override;                   // [6] wasm func 10232   name inferred (u18)
    bool Processa() override { return false; }         // [7] ICF 340: the urna threads run by themselves
    CMessageEleitor& GetFilaEleitor() override;        // [8] ICF 4708
    const comum::CAppState* GetEstadoAtual() override; // [9] ICF 4705
};

}  // namespace vota
