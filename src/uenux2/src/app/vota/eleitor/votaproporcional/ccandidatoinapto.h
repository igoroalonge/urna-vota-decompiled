// Reconstructed from vota_web_wasm.wasm (unit u39 = GetMensagemAudio; constructor/GetInst described by u06).
// Original (path inferred, as u19 did for cconfirmavotonominal.cpp): uenux2/src/app/vota/eleitor/votaproporcional/ccandidatoinapto.h
//
// vota::CCandidatoInapto: the number is a candidate who is not running ("inapto"): CONFIRMA makes it a NULL vote.
// Confirmation screen (CONFIRMA / CORRIGE) of a proportional cargo (Vereador, Deputado...). All the logic is
// in CConfirmaVotoEmCargo / CConfirmaProporcional (u06, u08); the leaf only fixes the screen, the RDV vote
// type and the sentence read to voters who vote with audio (slot 15).
//   constructor (inlined into the merged lazy-singleton body func 1961):
//       CConfirmaProporcional(ETelaVotacao 13 (CandidatoInapto), CVoto::ETipo 4 (nulo))
//
// RTTI: CVotacaoStateAudio <- CConfirmaVotoEmCargo <- CConfirmaProporcional <- vota::CCandidatoInapto
//   (typeinfo @1548512, vtable @1548440, 36 bytes, no own members)  [0] ICF 884 [1] ICF 883  [15] GetMensagemAudio 11748
#pragma once

#include <string>

#include "vota/eleitor/cconfirmavotoemcargo.h"

namespace vota {

class CCandidatoInapto final : public CConfirmaProporcional {
public:
    static CCandidatoInapto& GetInst();                             // merged body func 1961 (u06)

    std::string GetMensagemAudio() const override;      // [15] wasm func 11748

private:
    CCandidatoInapto();
};

}  // namespace vota
