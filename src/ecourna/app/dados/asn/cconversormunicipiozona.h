// ecourna-lib/ecourna/app/dados/asn/cconversormunicipiozona.h   (path inferred; already included by u11's
//   cconversoridentificacaosecaoeleitoral.cpp)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
#pragma once

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/app/dados/cidentificacaosecaoeleitoral.h"   // CMunicipioZona
#include "ModuloTiposEleitorais.h"

namespace ecourna::app::dados::asn {

// RTTI: CConversorMunicipioZona : IConversorASN<ModuloTiposEleitorais::MunicipioZona, CMunicipioZona>
// vtable @1130584: [0] 174  [1] 144  [2] 9131 DoConverte  [3] 9130 DoDeconverte
//   MunicipioZona ::= SEQUENCE { municipio INTEGER (1..99999), zona INTEGER (1..9999) }
class CConversorMunicipioZona
    : public api::asn::IConversorASN<ModuloTiposEleitorais::MunicipioZona, CMunicipioZona>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9131
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9130 (curated; not in u40)
};

}  // namespace ecourna::app::dados::asn
