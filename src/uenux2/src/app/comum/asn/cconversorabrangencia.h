// uenux2/src/app/comum/asn/cconversorabrangencia.h
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/md/cabrangencia.h"      // md::CAbrangencia (20 bytes): +0 ETipoAbrangencia tipo, +4 std::string uf,
                                        //                              +16 TMunicipioID municipio
#include "ModuloTiposEleitorais.h"

namespace comum::asn {

// Abrangencia = the geographic scope of a data file (candidates, parties...):
//   Abrangencia ::= SEQUENCE { tipo TipoAbrangencia {municipal(0), estadual(1), federal(2)},
//                              id IdentificacaoAbrangencia { siglaUF, codigoMunicipio OPTIONAL } OPTIONAL }
// RTTI: CConversorAbrangencia : IConversorASN<ModuloTiposEleitorais::Abrangencia, md::CAbrangencia>
// vtable @1561128: [0] 174 [1] 144 [2] 11462 DoConverte [3] 11461 DoDesconverte. sizeof 4 (vptr only).
class CConversorAbrangencia : public IConversorASN<ModuloTiposEleitorais::Abrangencia, md::CAbrangencia>
{
protected:
    TEntidade DoConverte(const TDado& abrangencia) const override;       // wasm func 11462 (srcloc line 45)
    TDado DoDesconverte(const TEntidade& abrangencia) const override;    // wasm func 11461
};

} // namespace comum::asn
