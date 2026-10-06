// Reconstructed from vota_web_wasm.wasm (unit u39; GetInst by u20).
// Original (path inferred): uenux2/src/app/vota/operador/outrasopcoes/cfimaquisicaovotos.h
// (entered from CConfirmaEncerramento, operador/outrasopcoes; its vtable sits between those of
// CConfirmaAudio and CConfirmaEncerramento).
//
// "Fim da aquisição de votos" = end of vote collection, the hinge of the ENCERRAMENTO (closing of the
// urna). Operator side: the mesário confirmed "Encerrar votação" (estadoVota is already 9
// FIMAQUISICAOVOTOS, set by CConfirmaEncerramento::ProcessInput). This transit state either starts the final
// registration of mesários or tells the voter thread to generate the BOLETIM DE URNA (message 7).
//
// RTTI: comum::CAppState <- vota::CFimAquisicaoVotos (typeinfo @1587256, vtable @1587220, 12 bytes)
//   [0] 174  [1] 144  [2] StartState 10737  [3..8] CAppState defaults
// GetInst = func 5428 = merged lazy-singleton body vota_f764(mutex @1905004, &s_inst @1905028, vtable, flags 0).
// Callers of GetInst: CConfirmaEncerramento::ProcessInput (10734), CAguardaInicio::ProcessMessage(10) (10217).
//
// WEB BUILD: dead code (operator thread not run) - which is exactly why the simulator never produces a BU
// (docs/10-boletim-de-urna.md §5.1).
#pragma once

#include "comum/cappstate.h"

namespace vota {

class CFimAquisicaoVotos final : public comum::CAppState {
public:
    static CFimAquisicaoVotos& GetInst();              // wasm func 5428

    void StartState() override;                        // [2] wasm func 10737

private:
    CFimAquisicaoVotos() : comum::CAppState(0) {}
};

}  // namespace vota
