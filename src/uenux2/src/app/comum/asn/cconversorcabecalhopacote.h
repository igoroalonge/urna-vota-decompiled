// uenux2/src/app/comum/asn/cconversorcabecalhopacote.h   (path inferred: its md class is comum/md/ccabecalhopacote.cpp,
// like CAbrangencia/CCabecalhoEntidade whose converters are attested in comum/asn/)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/md/ccabecalhopacote.h"
#include "ModuloTiposEleitorais.h"

namespace comum::asn {

// Header of a data package imported by the load system:
//   CabecalhoPacote ::= SEQUENCE { tipoPacote TipoPacote, idPacote IDPacote, nomepacote GeneralString,
//                                  versao NumericString (SIZE(12)), abrangencia [1] Abrangencia OPTIONAL, origem Sistema }
// md::CCabecalhoPacote (72 bytes; ctor comum/md/ccabecalhopacote.cpp:33/36/39): +0 md::ETipoPacote, +4 md::CIDPacote
//   (40 bytes, see util.cpp), +44 std::string nome, +56 std::string versao, +68 ESistemaJE origem.
//   There is no abrangência member: DoConverte never fills the optional field.
// RTTI: CConversorCabecalhoPacote : IConversorASN<ModuloTiposEleitorais::CabecalhoPacote, md::CCabecalhoPacote>
// vtable @1562744: [0] 174 [1] 144 [2] 11438 DoConverte [3] 11437 DoDesconverte (other unit). sizeof 4.
class CConversorCabecalhoPacote
    : public IConversorASN<ModuloTiposEleitorais::CabecalhoPacote, md::CCabecalhoPacote>
{
protected:
    TEntidade DoConverte(const TDado& cabecalho) const override;      // wasm func 11438
    TDado DoDesconverte(const TEntidade& cabecalho) const override;   // wasm func 11437 (util.cpp:89/329/451 inlined)
};

} // namespace comum::asn
