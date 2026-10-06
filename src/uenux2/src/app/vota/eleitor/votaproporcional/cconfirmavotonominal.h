// Reconstructed from vota_web_wasm.wasm (unit u39 = GetMensagemAudio; constructor/GetInst described by u06).
// Original (path inferred, as u19 did for cconfirmavotonominal.cpp): uenux2/src/app/vota/eleitor/votaproporcional/cconfirmavotonominal.h
//
// vota::CConfirmaVotoNominal: confirmation of a vote for a CANDIDATE (the full number matches a candidacy).
// Confirmation screen (CONFIRMA / CORRIGE) of a proportional cargo (Vereador, Deputado...). All the logic is
// in CConfirmaVotoEmCargo / CConfirmaProporcional (u06, u08); the leaf only fixes the screen, the RDV vote
// type and the sentence read to voters who vote with audio (slot 15).
//   constructor (inlined into the merged lazy-singleton body func 1961):
//       CConfirmaProporcional(ETelaVotacao 1 (Completa), CVoto::ETipo 2 (nominal))
//   slot 17 PosTecla = func 5925 (u19): CORRIGE here flushes the keypad buffer (IPoliticaExecucaoEleitor).
// RTTI: CVotacaoStateAudio <- CConfirmaVotoEmCargo <- CConfirmaProporcional <- vota::CConfirmaVotoNominal
//   (typeinfo @1549068, vtable @1548996, 36 bytes, no own members)  [0] ICF 884 [1] ICF 883  [15] GetMensagemAudio 11728
#pragma once

#include <string>

#include "vota/eleitor/cconfirmavotoemcargo.h"

namespace vota {

class CConfirmaVotoNominal final : public CConfirmaProporcional {
public:
    static CConfirmaVotoNominal& GetInst();                             // merged body func 1961 (u06)

    std::string GetMensagemAudio() const override;      // [15] wasm func 11728
    void PosTecla(int tecla) override;                  // [17] wasm func 5925 (u19; u06 header: TrataResultado)

private:
    CConfirmaVotoNominal();
};

}  // namespace vota
