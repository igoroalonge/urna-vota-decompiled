// uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/dados/asn/cconversorsecaoeleitoral.h"

#include <vector>

#include "comum/asn/util.h"
#include "comum/dados/asn/cconversoridentificacaoagregada.h"

namespace comum::asn {

// wasm func 11454 (vtable slot 2). The tool named it after the inlined IConversorASN<IdentificacaoAgregada>::Converte
// (iconversorasn.h:56, record 1561892). Not observed executing.
// The copy of the aggregated-section vector is a memcpy of (size - 2) bytes: the last element's 2 bytes of tail
// padding are not copied (member-wise copy of {int32, uint16}), not a bug.
ModuloLocal::SecaoEleitoral CConversorSecaoEleitoral::DoConverte(const md::CSecaoEleitoral& secao) const
{
    ModuloLocal::SecaoEleitoral entidade;
    entidade.set_tipo(Utils::ConverteTipoLocalVotacao(secao.GetTipo()));             // func 3801

    ModuloTiposEleitorais::IdentificacaoSecaoEleitoral id;
    ModuloTiposEleitorais::MunicipioZona municipioZona;
    municipioZona.set_municipio(secao.GetMunicipio());
    municipioZona.set_zona(secao.GetZona());
    id.set_municipioZona(municipioZona);
    id.set_local(secao.GetLocal());
    id.set_secao(secao.GetSecao());
    entidade.set_secao(id);

    const CConversorIdentificacaoAgregada conversor;
    if (secao.GetAgregadas().empty()) {
        entidade.omit_agregadas();                                                   // removeOptionalField(0)
        return entidade;
    }
    const std::vector<md::CIdentificacaoAgregada> agregadas = secao.GetAgregadas();   // by-value copy
    ASN1::SEQUENCE_OF<ModuloLocal::IdentificacaoAgregada> lista;
    for (const auto& agregada : agregadas) {
        lista.push_back(conversor.Converte(agregada));                               // DoConverte = 11456, :56
    }
    entidade.set_agregadas(lista);                                                   // includeOptionalField(0, 2)
    return entidade;
}

} // namespace comum::asn
