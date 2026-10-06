// uenux2/src/app/comum/gravadores/asn/cconversorentidadehashes.h   (path inferred: md class in
// gravadores/md/centidadehashes.cpp, attested converters of the same family in gravadores/asn/)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/gravadores/md/centidadehashes.h"
#include "api/hash/chasharquivo.h"      // api::hash::CHashArquivo (24 bytes: nome, hash/assinatura strings)
#include "comum/asn/cconversorhasharquivo.h"   // CConversorHashArquivo (path inferred by u17; vtable @1599000:
                                               //   [2] 10267 DoConverte, [3] 10269 DoDesconverte)
#include "ModuloHashes.h"

namespace comum::asn {

// hash.dat of the result medium:
//   EntidadeHashes ::= SEQUENCE { cabecalho CabecalhoEntidade, fase Fase, siglaUF SiglaUF,
//                                 identificacaoUrna IdentificacaoUrna OPTIONAL, versaoUENUX GeneralString,
//                                 hashesArquivos SEQUENCE OF ArquivoAssinatura {nomeArquivo, assinatura} }
// md::CEntidadeHashes: +0 CCabecalhoEntidade (20), +20 EUrnaFase, +24 std::string siglaUF,
//   +36 std::optional<md::CIdentificacaoSecao> (20 bytes: vptr of md::CIdentificacaoUrna, +40 municipio, +44 zona,
//       +48 local, +52 secao; engaged flag +56),
//   +60 std::optional<md::CIdentificacaoUrnaContingencia> (vptr, +64 municipio, +68 zona; flag +72),
//   +76 std::string versaoUENUX, +88 std::vector<api::hash::CHashArquivo>.
// RTTI: CConversorEntidadeHashes : IConversorASN<ModuloHashes::EntidadeHashes, md::CEntidadeHashes>
// vtable @1599092: [0] 174 [1] 144 [2] 10266 DoConverte [3] 10265 (base DoDesconverte, throws 7656). sizeof 4.
class CConversorEntidadeHashes : public IConversorASN<ModuloHashes::EntidadeHashes, md::CEntidadeHashes>
{
protected:
    TEntidade DoConverte(const TDado& hashes) const override;   // wasm func 10266
};


} // namespace comum::asn
