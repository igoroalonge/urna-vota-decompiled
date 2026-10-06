// FRAGMENT of ecourna-lib/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.cpp (path inferred; the
// class and DoDeconverte are reconstructed by unit u11). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include "ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.h"

#include "ecourna/app/dados/asn/cconversormunicipiozona.h"

namespace ecourna::app::dados::asn {

// wasm func 9129 (vtable @1130788 slot 2). Caller: CConversorComparecimentoSecao::DoConverte (9126) through
// IConversorASN<IdentificacaoSecaoEleitoral, ...>::Converte (func 9124).
ModuloTiposEleitorais::IdentificacaoSecaoEleitoral
CConversorIdentificacaoSecaoEleitoral::DoConverte(const CIdentificacaoSecaoEleitoral& dado) const
{
    ModuloTiposEleitorais::IdentificacaoSecaoEleitoral entidade;
    const CConversorMunicipioZona conversorMunicipioZona;                                // vptr @1130584
    // The 8-byte CMunicipioZona is copied to the stack, then Converte (func 9128 -> 9131) validates the result.
    entidade.set_municipioZona(conversorMunicipioZona.Converte(dado.GetMunicipioZona()));
    entidade.set_local(dado.GetLocal());                                                 // +8, 32-bit
    entidade.set_secao(dado.GetSecao());                                                 // +12, 16-bit
    return entidade;
}

}  // namespace ecourna::app::dados::asn
