// uenux2/src/app/comum/gravadores/asn/cconversorentidadehashes.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/gravadores/asn/cconversorentidadehashes.h"

#include "comum/asn/cconversorcabecalhoentidade.h"
#include "comum/asn/util.h"

namespace comum::asn {

// wasm func 10266 (vtable slot 2). The tool named it IConversorASN<ArquivoAssinatura, CHashArquivo>::Converte after
// the inlined per-file conversion (iconversorasn.h:56, record 1599176). Called by CGravadorHashes when the result
// files are written at the end of the day (hash.dat); not observed executing in the recorded votes.
ModuloHashes::EntidadeHashes CConversorEntidadeHashes::DoConverte(const md::CEntidadeHashes& hashes) const
{
    ModuloHashes::EntidadeHashes entidade;
    entidade.set_cabecalho(CConversorCabecalhoEntidade().Converte(hashes.GetCabecalho()));   // thunk 2275
    entidade.set_fase(Utils::ConverteFase(hashes.GetFase()));
    entidade.set_siglaUF(hashes.GetSiglaUF());
    entidade.set_versaoUENUX(hashes.GetVersaoUENUX());

    if (hashes.GetIdentificacaoSecao().has_value()) {                   // section urna
        const auto& secao = *hashes.GetIdentificacaoSecao();
        ModuloTiposEleitorais::IdentificacaoSecaoEleitoral id;
        id.ref_municipioZona().set_municipio(secao.GetMunicipio());
        id.ref_municipioZona().set_zona(secao.GetZona());
        id.set_local(secao.GetLocal());
        id.set_secao(secao.GetSecao());
        entidade.ref_identificacaoUrna().select_identificacaoSecaoEleitoral() = id;   // CHOICE alt 0
    } else if (hashes.GetIdentificacaoContingencia().has_value()) {     // contingency (spare) urna
        const auto& contingencia = *hashes.GetIdentificacaoContingencia();
        ModuloTiposEleitorais::IdentificacaoContingencia id;
        id.ref_municipioZona().set_municipio(contingencia.GetMunicipio());
        id.ref_municipioZona().set_zona(contingencia.GetZona());
        entidade.ref_identificacaoUrna().select_identificacaoContingencia() = id;     // CHOICE alt 1
    } else {
        entidade.omit_identificacaoUrna();
    }

    const CConversorHashArquivo conversor;
    for (const api::hash::CHashArquivo& hash : hashes.GetHashes()) {
        entidade.ref_hashesArquivos().push_back(conversor.Converte(hash));   // Converte inlined: 7653 on invalid
    }
    return entidade;
}

} // namespace comum::asn
