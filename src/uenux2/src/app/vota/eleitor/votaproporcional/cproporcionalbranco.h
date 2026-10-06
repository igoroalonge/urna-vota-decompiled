// Reconstructed from vota_web_wasm.wasm (unit u39 = GetMensagemAudio; constructor/GetInst described by u06).
// Original (path inferred, as u19 did for cconfirmavotonominal.cpp): uenux2/src/app/vota/eleitor/votaproporcional/cproporcionalbranco.h
//
// vota::CProporcionalBranco: BRANCO (blank vote) pressed in a proportional cargo.
// Confirmation screen (CONFIRMA / CORRIGE) of a proportional cargo (Vereador, Deputado...). All the logic is
// in CConfirmaVotoEmCargo / CConfirmaProporcional (u06, u08); the leaf only fixes the screen, the RDV vote
// type and the sentence read to voters who vote with audio (slot 15).
//   constructor (inlined into the merged lazy-singleton body func 1961):
//       CConfirmaProporcional(ETelaVotacao 3 (VotoBranco), CVoto::ETipo 3 (branco))
//
// RTTI: CVotacaoStateAudio <- CConfirmaVotoEmCargo <- CConfirmaProporcional <- vota::CProporcionalBranco
//   (typeinfo @1549716, vtable @1549644, 36 bytes, no own members)  [0] ICF 884 [1] ICF 883  [15] GetMensagemAudio 11704
#pragma once

#include <string>

#include "vota/eleitor/cconfirmavotoemcargo.h"

namespace vota {

class CProporcionalBranco final : public CConfirmaProporcional {
public:
    static CProporcionalBranco& GetInst();                             // merged body func 1961 (u06)

    std::string GetMensagemAudio() const override;      // [15] wasm func 11704

private:
    CProporcionalBranco();
};

}  // namespace vota
