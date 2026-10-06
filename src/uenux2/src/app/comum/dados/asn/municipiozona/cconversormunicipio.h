// uenux2/src/app/comum/dados/asn/municipiozona/cconversormunicipio.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/municipiozona/cmunicipio.h"   // md::CMunicipio: +0 codigo, +4 std::string nome, +16 bool comBiometria
#include "ModuloTiposCadastro.h"

namespace comum::asn {

// Municipio ::= SEQUENCE { codigo INTEGER (1..99999), nome GeneralString (SIZE(1..35)), capital BOOLEAN,
//                          codigoIBGE [1] INTEGER OPTIONAL, comBiometria [2] BOOLEAN OPTIONAL }
// RTTI: CConversorMunicipio : IConversorASN<ModuloTiposCadastro::Municipio, md::CMunicipio>
// vtable @1569680: [0] 174 [1] 144 [2] 11379 DoConverte [3] 11378 DoDesconverte (u05-foreign-fragments.cpp). sizeof 4.
class CConversorMunicipio : public IConversorASN<ModuloTiposCadastro::Municipio, md::CMunicipio>
{
protected:
    TEntidade DoConverte(const TDado& municipio) const override;      // wasm func 11379
    TDado DoDesconverte(const TEntidade& municipio) const override;   // wasm func 11378 (unit u05)
};

} // namespace comum::asn
