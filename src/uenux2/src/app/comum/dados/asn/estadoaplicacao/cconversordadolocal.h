// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/estadoaplicacao/cdadolocal.h"
#include "ModuloEstadoGeralUrna.h"

namespace comum::asn {

// Part of eg.bin (EstadoGeralUrna, the urna's general state):
//   DadoLocal ::= SEQUENCE { uf SiglaUF, secaoCarga DadoSecao {municipio, zona, secao}, tipoLocalVotacao TipoLocalVotacao }
// md::estadoaplicacao::CDadoLocal (24 bytes): +0 std::string uf, +12 CLocalidadeEleitoral (8: municipio uint32,
//   zona uint16, secao uint16), +20 ETipoLocalVotacao.
// RTTI: CConversorDadoLocal : IConversorASN<ModuloEstadoGeralUrna::DadoLocal, md::estadoaplicacao::CDadoLocal>
// vtable @1567448: [0] 174 [1] 144 [2] 11403 DoConverte [3] 11404 DoDesconverte (other unit). sizeof 4.
class CConversorDadoLocal
    : public IConversorASN<ModuloEstadoGeralUrna::DadoLocal, md::estadoaplicacao::CDadoLocal>
{
protected:
    TEntidade DoConverte(const TDado& local) const override;      // wasm func 11403
    TDado DoDesconverte(const TEntidade& local) const override;   // wasm func 11404 (other unit)
};

} // namespace comum::asn
