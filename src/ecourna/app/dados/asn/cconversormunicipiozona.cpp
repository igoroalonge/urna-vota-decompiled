// ecourna-lib/ecourna/app/dados/asn/cconversormunicipiozona.cpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40 (DoConverte). DoDeconverte (func 9130) belongs to no unit
// reconstruction yet; it is written here too so the file is complete.
#include "ecourna/app/dados/asn/cconversormunicipiozona.h"

namespace ecourna::app::dados::asn {

// wasm func 9131 (vtable slot 2). Plain value copies: municipio (32-bit, +0) and zona (16-bit, +4). The
// ASN.1 lower bounds (1) are checked later by IConversorASN::Converte (isStrictlyValid), not here.
ModuloTiposEleitorais::MunicipioZona CConversorMunicipioZona::DoConverte(const CMunicipioZona& dado) const
{
    ModuloTiposEleitorais::MunicipioZona entidade;
    entidade.set_municipio(dado.GetMunicipio());
    entidade.set_zona(dado.GetZona());
    return entidade;
}

// wasm func 9130 (vtable slot 3; name curated by the tools, not part of u40). The CBaseType constructors
// (2851 "MunicipioID" 0..99999, 5850 "ZonaID" 0..9999) throw CPatternError 1300 when out of range.
CMunicipioZona CConversorMunicipioZona::DoDeconverte(const ModuloTiposEleitorais::MunicipioZona& entidade) const
{
    return CMunicipioZona(TMunicipioID(entidade.get_municipio()), TZonaID(entidade.get_zona()));   // func 5108
}

}  // namespace ecourna::app::dados::asn
