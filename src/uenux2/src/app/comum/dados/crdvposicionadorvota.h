// uenux2/src/app/comum/dados/crdvposicionadorvota.h   (path inferred: RTTI only; created by the CRdvVota constructor
// of the attested dados/crdvvota.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// RTTI: comum::CRdvPosicionadorVota : comum::md::CRdvPosicionador, vtable @1560252
//   [0] 174 (trivial dtor) [1] 144 (deleting) [2] 11496 Posiciona. sizeof 4 (vptr only).
// The positioner decides WHERE in the list of votes of a cargo (md::CVotos, a vector of 16-byte md::CVoto
// {+0 ETipoVoto tipo, +4 std::string digitado}) a new vote is inserted by md::CVotosCargos::Insere
// (cvotoscargos.cpp). Inserting at a position that depends only on the vote's content keeps the RDV sorted, so the
// order of the votes in rdv.dat carries no information about the order in which voters voted.
#pragma once

#include <cstddef>

#include "comum/dados/md/rdv/cvotoscargos.h"   // md::CRdvPosicionador, md::CVotos, md::CVoto

namespace comum {

class CRdvPosicionadorVota : public md::CRdvPosicionador
{
public:
    std::size_t Posiciona(const md::CVotos& votos, const md::CVoto& voto) const override;   // wasm func 11496
};

} // namespace comum
