// Reconstructed from vota_web_wasm.wasm (unit u39 = StartState; ProcessMessage by u19).
// Original (path inferred): uenux2/src/app/vota/eleitor/csincronismoeleitor.h
// (include path already used by other units; the class sits between CFimVotoEleitor and
// impl::CSincronismoVotoEleitor in the data, i.e. with the voter-thread states of vota/eleitor).
//
// "Sincronismo do eleitor": the voter confirmed the last cargo and the ballot is in the in-memory RDV
// (CEleitorVotando::GravaVotos, func 4454). The voter screen shows "Gravando" with a 4-step progress bar
// while the two threads synchronise:
//   1. StartState (here): message 13 to the operator thread; show the progress screen.
//   2. Operator: CSincronismoOperador marks the voter as VOTOU and posts message 5 back.
//   3. ProcessMessage(5) (func 7181, u19): ISincronismoVotoEleitor::SincronizaVoto() makes the vote durable
//      (RDV files on internal and external flash, advancing the bar), then -> CFimVotoEleitor ("FIM").
// In the web build step 2 never happens (operator thread not run): votaTick posts message 5 itself
// (vota_web_wasm.u30.cpp) and SincronizaVoto is the no-op CSincronismoVotoEleitorWeb.
//
// RTTI: comum::CAppState <- vota::CSincronismoEleitor (typeinfo @1534120, vtable @1534084, 12 bytes)
//   [0] 174 [1] 144 [2] StartState 7178 [3] 7480 [4] 1661 [5] nop [6] ProcessMessage 7181 [7] nop [8] nop
// Lazy singleton @1833160, CAppState(1 = messages), GetInst + ctor inlined into func 4454.
#pragma once

#include <cstdint>

#include "comum/cappstate.h"

namespace vota {

using uebyte = std::uint8_t;

class CSincronismoEleitor final : public comum::CAppState {
public:
    static CSincronismoEleitor& GetInst();             // inlined into CEleitorVotando::GravaVotos (4454)

    void StartState() override;                        // [2] wasm func 7178 - observed executing
    void ProcessMessage(uebyte mensagem) override;     // [6] wasm func 7181 (u19)

private:
    CSincronismoEleitor() : comum::CAppState(1) {}     // shared_f224(this, 1)
};

}  // namespace vota
