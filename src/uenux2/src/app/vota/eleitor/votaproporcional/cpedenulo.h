// uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.h   (path inferred from cpedenulo.cpp, attested by the
// std::source_location record :44)
// Reconstructed from vota_web_wasm.wasm (unit u26).
//
// vota::CPedeNulo: proportional vote whose two first digits are not a valid party for this office. The
// voter still completes the number (screen ETelaVotacao 5, "Número errado"); the result is a null vote,
// unless the full number is an "inapto" candidate (then the "candidato inapto" conferência).
//
// RTTI: CCompletaProporcional <- vota::CPedeNulo (typeinfo @1548616, vtable @1548532), 32 bytes.
//   [0] 884 / [1] 883 (ICF dtors)  [15] GetMensagemAudio 11745  [16] GetProximoEstado 11744
#pragma once

#include <string>

#include "vota/eleitor/votaproporcional/ccompletaproporcional.h"

namespace vota {

class CPedeNulo : public CCompletaProporcional {
public:
    /// Inlined into CPedeProporcional::ProcessInputAudio (func 11711): lazy singleton @1837788 (mutex
    /// residue @1837764), new(32), CCompletaProporcional(ETelaVotacao 5).                name inferred
    static CPedeNulo& GetInst();

    std::string GetMensagemAudio() const override;                                  // [15] func 11745
    comum::CAppState* GetProximoEstado(const std::string& numero) const override;   // [16] func 11744 (:44)

private:
    CPedeNulo() : CCompletaProporcional(ETelaVotacao(5)) {}
};

} // namespace vota
