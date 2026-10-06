// Reconstructed from vota_web_wasm.wasm (unit u39 = GetMensagemAudio; GetProximoEstado by u04).
// Original (path inferred; included by cpedeproporcional.cpp as "vota/eleitor/votaproporcional/cpedenominal.h"):
// uenux2/src/app/vota/eleitor/votaproporcional/cpedenominal.h
//
// vota::CPedeNominal ("pede nominal"): proportional vote whose two first digits are a valid party - the voter
// goes on typing the candidate's remaining digits (screen ETelaVotacao 8). CONFIRMA with only the party
// digits is a party vote; a complete number leads to the conferência of a candidate / non-existent
// candidate / inapto candidate (slot 16 GetProximoEstado, func 11724). Observed in the recorded vote
// (Vereador 91001) - but this function (slot 15) only runs when the voter uses audio.
//
// RTTI: CVotacaoStateAudio <- CCompletaProporcional <- vota::CPedeNominal (typeinfo @1549156, vtable @1549088),
// 32 bytes  [0] ICF 884 [1] ICF 883 [15] GetMensagemAudio 11725 [16] GetProximoEstado 11724 (u04)
#pragma once

#include <string>

#include "vota/eleitor/votaproporcional/ccompletaproporcional.h"

namespace vota {

class CPedeNominal final : public CCompletaProporcional {
public:
    /// Inlined into CPedeProporcional::ProcessInputAudio (11711): lazy singleton @1837972, new(32).
    static CPedeNominal& GetInst();

    std::string GetMensagemAudio() const override;                                  // [15] wasm func 11725
    comum::CAppState* GetProximoEstado(const std::string& numero) const override;   // [16] wasm func 11724 (u04)

private:
    CPedeNominal() : CCompletaProporcional(ETelaVotacao(8)) {}
};

}  // namespace vota
