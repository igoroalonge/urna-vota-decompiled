// Reconstructed from vota_web_wasm.wasm (unit u39 = StartState; ProcessInput by u02).
// Original (path inferred by u02): uenux2/src/app/vota/eleitor/cinspecionaurna.h
//
// Voter-terminal side of the periodic booth inspection. CAguardaMensagem receives message 12 from the
// operator thread (CPedeIdentidade::ProcessTick, when the drawn inspection time has come) and enters this
// state: "Por favor, inspecione cabina e urna." / key CONFIRMA "Continuar". CONFIRMA -> CUrnaInspecionada,
// log "Inspeção da urna confirmada", message 11 to the operator (CAguardaInspecao -> CConfirmaInspecionada).
//
// RTTI: comum::CAppState <- vota::CInspecionaUrna (typeinfo @1547492, vtable @1547456, 20 bytes)
//   [0] ICF 244 [1] ICF 387 [2] StartState 11798 [7] ProcessInput 11797 (u02)
// Lazy singleton @1837704, CAppState(2), m_form = CTelasVota::CriaTelaTextoConfirmaCorrige(...) (u02).
//
// WEB BUILD: unreachable - only the operator thread posts message 12.
#pragma once

#include "comum/cappstate.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

class CInspecionaUrna final : public comum::CAppState {
public:
    static CInspecionaUrna& GetInst();

    void StartState() override;                        // [2] wasm func 11798
    void ProcessInput() override;                      // [7] wasm func 11797 (u02)

private:
    CInspecionaUrna();

    CFormInterativoTelaVota m_form;                    // +12 (+16)
};

}  // namespace vota
