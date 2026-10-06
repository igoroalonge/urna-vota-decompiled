// Reconstructed from vota_web_wasm.wasm (unit u39 = GetMensagemAudio; constructor/GetInst described by u06).
// Original (path inferred, as u19 did for cconfirmavotonominal.cpp): uenux2/src/app/vota/eleitor/votaproporcional/ccandidatoinexistente.h
//
// vota::CCandidatoInexistente: full number typed, valid party but no such candidate: CONFIRMA counts it for the PARTY.
// Confirmation screen (CONFIRMA / CORRIGE) of a proportional cargo (Vereador, Deputado...). All the logic is
// in CConfirmaVotoEmCargo / CConfirmaProporcional (u06, u08); the leaf only fixes the screen, the RDV vote
// type and the sentence read to voters who vote with audio (slot 15).
//   constructor (inlined into the merged lazy-singleton body func 1961):
//       CConfirmaProporcional(ETelaVotacao 11 (CandidatoInexistente), CVoto::ETipo 1 (legenda))
//
// RTTI: CVotacaoStateAudio <- CConfirmaVotoEmCargo <- CConfirmaProporcional <- vota::CCandidatoInexistente
//   (typeinfo @1548976, vtable @1548904, 36 bytes, no own members)  [0] ICF 884 [1] ICF 883  [15] GetMensagemAudio 11731
#pragma once

#include <string>

#include "vota/eleitor/cconfirmavotoemcargo.h"

namespace vota {

class CCandidatoInexistente final : public CConfirmaProporcional {
public:
    static CCandidatoInexistente& GetInst();                             // merged body func 1961 (u06)

    std::string GetMensagemAudio() const override;      // [15] wasm func 11731

private:
    CCandidatoInexistente();
};

}  // namespace vota
