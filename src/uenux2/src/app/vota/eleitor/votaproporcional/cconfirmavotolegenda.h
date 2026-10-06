// Reconstructed from vota_web_wasm.wasm (unit u39 = GetMensagemAudio; constructor/GetInst described by u06).
// Original (path inferred, as u19 did for cconfirmavotonominal.cpp): uenux2/src/app/vota/eleitor/votaproporcional/cconfirmavotolegenda.h
//
// vota::CConfirmaVotoLegenda: confirmation of a PARTY vote ("voto de legenda": only the 2 party digits were typed).
// Confirmation screen (CONFIRMA / CORRIGE) of a proportional cargo (Vereador, Deputado...). All the logic is
// in CConfirmaVotoEmCargo / CConfirmaProporcional (u06, u08); the leaf only fixes the screen, the RDV vote
// type and the sentence read to voters who vote with audio (slot 15).
//   constructor (inlined into the merged lazy-singleton body func 1961):
//       CConfirmaProporcional(ETelaVotacao 9 (VotoLegendaIncompleto), CVoto::ETipo 1 (legenda))
//
// RTTI: CVotacaoStateAudio <- CConfirmaVotoEmCargo <- CConfirmaProporcional <- vota::CConfirmaVotoLegenda
//   (typeinfo @1548884, vtable @1548812, 36 bytes, no own members)  [0] ICF 884 [1] ICF 883  [15] GetMensagemAudio 11735
#pragma once

#include <string>

#include "vota/eleitor/cconfirmavotoemcargo.h"

namespace vota {

class CConfirmaVotoLegenda final : public CConfirmaProporcional {
public:
    static CConfirmaVotoLegenda& GetInst();                             // merged body func 1961 (u06)

    std::string GetMensagemAudio() const override;      // [15] wasm func 11735

private:
    CConfirmaVotoLegenda();
};

}  // namespace vota
