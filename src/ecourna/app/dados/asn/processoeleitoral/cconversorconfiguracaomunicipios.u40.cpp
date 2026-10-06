// FRAGMENT of ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipios.cpp (path
// inferred; class + DoDeconverte by unit u11). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include <vector>

#include "ecourna/app/dados/asn/cconversorcabecalhoentidade.h"
#include "ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.h"

namespace ecourna::app::dados::asn {

// wasm func 9141 (vtable @1129768 slot 2). The inverse of DoDeconverte (9137): the map keyed by municipality is
// flattened (in key order) into a vector of 40-byte CConfiguracaoMunicipio (push_back with the reallocation
// inlined), which ConverteLista (thunk 9140) turns into the SEQUENCE OF; the generated setter assigns it by
// copy-and-swap (SEQUENCE_OF(first, last) = thunk 9139, then SEQUENCE_OF_Base::clear of the temporary).
ModuloConfiguracaoMunicipios::EntidadeConfiguracaoMunicipios
CConversorConfiguracaoMunicipios::DoConverte(const CConfiguracaoMunicipios& dado) const
{
    const CConversorCabecalhoEntidade conversorCabecalho;          // vptr @1123740
    const CConversorConfiguracaoMunicipio conversorMunicipio;      // vptr @1129456

    ModuloConfiguracaoMunicipios::EntidadeConfiguracaoMunicipios entidade;
    entidade.set_cabecalho(conversorCabecalho.Converte(dado.GetCabecalho()));      // func 9211 -> 9219

    std::vector<CConfiguracaoMunicipio> configuracoes;
    for (const auto& [codigo, configuracao] : dado.GetConfiguracoes())
        configuracoes.push_back(configuracao);

    entidade.set_configuracoes(api::asn::ConverteLista(configuracoes, conversorMunicipio));
    return entidade;
}

// wasm func 9139 (table slot 6846): ASN1::SEQUENCE_OF<ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio>::
// SEQUENCE_OF(first, last), a thunk into the merged template body 2926 (element vtable @1130260, SEQUENCE_OF
// vtable @1130112, info @1130072). Library template instantiation.

}  // namespace ecourna::app::dados::asn
