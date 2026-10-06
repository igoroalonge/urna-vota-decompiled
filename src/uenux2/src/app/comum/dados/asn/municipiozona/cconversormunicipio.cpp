// uenux2/src/app/comum/dados/asn/municipiozona/cconversormunicipio.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21). DoDesconverte (wasm 11378) is in
// src/uenux2/src/app/comum/dados/u05-foreign-fragments.cpp.
#include "comum/dados/asn/municipiozona/cconversormunicipio.h"

namespace comum::asn {

// wasm func 11379 (vtable slot 2). Called when the urna writes its Local (-lo.dat, CConversorLocal::DoConverte) and
// by CConversorDadosDisponiveisCarga::DoConverte. Not observed executing.
// md::CMunicipio has no "capital" and no IBGE code: capital is always written as FALSE and codigoIBGE is absent.
ModuloTiposCadastro::Municipio CConversorMunicipio::DoConverte(const md::CMunicipio& municipio) const
{
    ModuloTiposCadastro::Municipio entidade;
    entidade.set_codigo(municipio.GetCodigo());   // through a Constrained_INTEGER<1, 99999> temporary (@1567692)
    entidade.set_nome(municipio.GetNome());
    entidade.set_capital(false);
    entidade.set_comBiometria(municipio.GetComBiometria());   // includeOptionalField(1, 4)
    return entidade;
}

} // namespace comum::asn
