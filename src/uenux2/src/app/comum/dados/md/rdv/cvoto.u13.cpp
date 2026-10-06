// uenux2/src/app/comum/dados/md/rdv/cvoto.cpp  --  FRAGMENT written by unit u13
// (the file is known from the srcloc of CVoto::CVoto(ETipo, const std::string&), func 2256, line 80).
//
// md::CVoto = { ETipo m_tipo +0; std::string m_conteudo +4 } (16 bytes). ETipo follows the RDV ASN.1
// TipoVoto: legenda 1, nominal 2, branco 3, nulo 4, brancoAposSuspensao 5, nuloAposSuspensao 6,
// nuloPorRepeticao 7, nuloCargoSemCandidato 8, nuloAposSuspensaoCargoSemCandidato 9.
#include "comum/dados/md/rdv/cvoto.h"

namespace comum::md {

// wasm func 5645 - name inferred. "Null votes that count as NULO in the tally": 4, 6 and 7.
// Compiled as "tipo == 7 || (tipo & ~2) == 4". The cargo-without-candidate nulls (8, 9) are not included.
bool CVoto::EhNulo(ETipo tipo)
{
    return tipo == ETipo::NULO || tipo == ETipo::NULO_APOS_SUSPENSAO || tipo == ETipo::NULO_POR_REPETICAO;
}

// wasm func 11316 - name inferred. Passed as a pointer-to-member {slot 2833, adj 0} to the counting
// helper (func 2897) of CVotosEleicoesVota::Nulos(TCargoID)'s lambda; also used by func 11514.
bool CVoto::EhNulo() const
{
    return EhNulo(m_tipo);
}

} // namespace comum::md
