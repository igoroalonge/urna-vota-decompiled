// uenux2/src/app/comum/dados/md/rdv/cvoto.cpp  --  FRAGMENT written by unit u36 (see also cvoto.u13.cpp).
// Reconstructed from vota_web_wasm.wasm.
//
// md::CVoto = { ETipo m_tipo +0; std::string m_conteudo +4 } (16 bytes). ETipo follows the RDV ASN.1
// TipoVoto: legenda 1, nominal 2, branco 3, nulo 4, brancoAposSuspensao 5, nuloAposSuspensao 6,
// nuloPorRepeticao 7, nuloCargoSemCandidato 8, nuloAposSuspensaoCargoSemCandidato 9.
#include "comum/dados/md/rdv/cvoto.h"

namespace comum::md {

// wasm func 11315 (table slot 2834)                                              // name inferred
// "Is this a blank vote?" A blank vote typed normally (BRANCO) or recorded as blank after the voter was
// suspended (BRANCO_APOS_SUSPENSAO) both count as BRANCO in the tally.
// Used as a pointer to member function {slot 2834, adj 0}: CVotosEleicoesVota::Brancos(TCargoID)
// (cvotoseleicoesvota.cpp:184, wasm 3747 = CRdvVota vtable slot 9) builds a std::function lambda whose
// body (wasm 2897) calls CVotos::TotalQue(&CVoto::EhBranco) (wasm 11340). That counter feeds the BU
// ("brancos"/"BRAN" of each cargo) and the printed reports' trailers (11980/11981).
// Siblings: EhNulo 11316 (types 4, 6, 7), EhLegenda 11317 (== 1), EhNominal 11318 (== 2).
bool CVoto::EhBranco() const
{
    return m_tipo == ETipo::BRANCO || m_tipo == ETipo::BRANCO_APOS_SUSPENSAO;
}

}  // namespace comum::md
