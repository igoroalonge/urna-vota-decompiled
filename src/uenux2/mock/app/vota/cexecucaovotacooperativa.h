// Reconstructed from vota_web_wasm.wasm (unit u39 = constructor and slot 7; slots 2/3 by unit u18). All three
// functions are defined in cexecucaovotacooperativa.cpp (the database files them there, component app:mock).
// Original (path inferred): uenux2/mock/app/vota/cexecucaovotacooperativa.h
//   Why mock/app/vota: the class exists only for the web build (no pthreads: "cooperative" execution), its
//   vtable @1532252 is emitted right after the last mock/app/simulador/wasm class (simulador::CWasmTimerScheduler)
//   and right before the first src/app/vota class (vota::CAssinadorVota), and the mock tree mirrors src/app
//   (mock/app/comum/cappinfobuilder.cpp is attested). u18 first proposed mock/app/simulador/wasm/; u30 includes it
//   as "vota/cexecucaovotacooperativa.h". Either way it is a mock, not urna code.
//
// The web replacement of vota::CExecucaoVota. main() (vota_web_wasm.cpp) registers (CPolySingletonList::replace)
//     std::make_unique<vota::CExecucaoVotaCooperativa>(vota::CAguardaMensagem::GetInst())
// as the IExecucaoVota poly-singleton. Nothing runs in a thread: votaInit calls Inicia (slot 3) once (install
// the first voter state), and every votaTick (once per requestAnimationFrame, ~60/s) calls Processa(),
// i.e. one iteration of the voter thread's loop. The operator and monitor threads are never stepped.
//
// RTTI: vota::IExecucaoVota <- vota::CExecucaoVotaCooperativa (typeinfo @1532292, vtable @1532252, 8 bytes)
//   [0] 174 [1] 144 [2] 4713 [3] 4713 (same body) [4] [5] [6] nop 218 [7] Processa 7823
//   [8] ICF 4708 (&CThreadEleitor +36) [9] ICF 4705 (current voter state)
#pragma once

#include "comum/cappstate.h"
#include "vota/iexecucaovota.h"

namespace vota {

class CExecucaoVotaCooperativa final : public IExecucaoVota {
public:
    explicit CExecucaoVotaCooperativa(comum::CAppState& estadoInicial);   // wasm func 7828

    void Executa() override;                           // [2] wasm func 4713 (u18)
    void Inicia() override;                            // [3] wasm func 4713 (u18, same body) - called by votaInit
    void Aguarda() override {}                         // [4] nop
    void IniciaOperador() override {}                  // [5] nop
    void FinalizaThreads() override {}                 // [6] nop
    bool Processa() override;                          // [7] wasm func 7823 - observed executing (every votaTick)
    CMessageEleitor& GetFilaEleitor() override;        // [8] ICF 4708
    const comum::CAppState* GetEstadoAtual() override; // [9] ICF 4705

private:
    comum::CAppState& m_estadoInicial;                 // +4 CAguardaMensagem
};

}  // namespace vota
