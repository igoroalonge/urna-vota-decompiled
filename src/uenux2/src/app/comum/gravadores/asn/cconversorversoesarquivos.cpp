// uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/gravadores/asn/cconversorversoesarquivos.h"

namespace comum::asn {

// wasm func 10264 (vtable slot 2). Not observed executing. The file list is iterated in std::map order (sorted by
// file name). Each element is built on the stack, cloned into the SEQUENCE OF (do_clone, AbstractData slot 3) and
// the temporary list is then copy-assigned into the entity (copy + swap).
ModuloVersaoArquivos::EntidadeVersaoArquivos CConversorVersoesArquivos::DoConverte(const md::CVersoesArquivos& versoes) const
{
    ModuloVersaoArquivos::EntidadeVersaoArquivos entidade;
    entidade.set_versaoTag(versoes.GetVersaoTag());
    ASN1::SEQUENCE_OF<ModuloTiposEleitorais::ArquivoAssinatura> arquivos;
    for (const auto& [nome, assinatura] : versoes.GetArquivos()) {
        ModuloTiposEleitorais::ArquivoAssinatura arquivo;
        arquivo.set_nomeArquivo(nome);
        arquivo.set_assinatura(assinatura);
        arquivos.push_back(arquivo);
    }
    entidade.set_arquivos(arquivos);
    return entidade;
}

} // namespace comum::asn
