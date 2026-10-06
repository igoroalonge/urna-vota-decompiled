// uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.h   (path inferred; included by u03's cconversorlocal.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/csecaoeleitoral.h"
#include "ModuloLocal.h"

namespace comum::asn {

// SecaoEleitoral ::= SEQUENCE { tipo TipoLocalVotacao, secao IdentificacaoSecaoEleitoral {municipioZona {municipio,
//                               zona}, local, secao}, agregadas SEQUENCE OF IdentificacaoAgregada OPTIONAL }
// ("seção agregada" = a small section merged into this one: its voters vote on this urna.)
// md::CSecaoEleitoral (36 bytes): +0 ETipoLocalVotacao, +8 municipio, +12 zona (uint16), +16 local, +20 secao
//   (uint16), +24 std::vector<md::CIdentificacaoAgregada> (8-byte elements {+0 ETipoLocalVotacao, +4 uint16 numero}).
// RTTI: CConversorSecaoEleitoral : IConversorASN<ModuloLocal::SecaoEleitoral, md::CSecaoEleitoral>
// vtable @1561712: [0] 174 [1] 144 [2] 11454 DoConverte [3] 11453 DoDesconverte (other unit). sizeof 4.
class CConversorSecaoEleitoral : public IConversorASN<ModuloLocal::SecaoEleitoral, md::CSecaoEleitoral>
{
protected:
    TEntidade DoConverte(const TDado& secao) const override;      // wasm func 11454
    TDado DoDesconverte(const TEntidade& secao) const override;   // wasm func 11453 (other unit)
};

} // namespace comum::asn
