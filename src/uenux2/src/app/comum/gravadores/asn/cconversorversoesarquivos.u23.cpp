// uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.cpp   (path inferred)  --  FRAGMENT of unit u23
// Reconstructed from vota_web_wasm.wasm. DoConverte (func 10264) and the class declaration are in
// cconversorversoesarquivos.cpp/.h (unit u21); this file adds the slot-3 method that was assigned to unit u23.
#include "comum/gravadores/asn/cconversorversoesarquivos.h"

#include <map>
#include <string>

namespace comum::asn {

// wasm func 10263 (vtable slot 3 of CConversorVersoesArquivos). Not observed executing.
// ASN.1 EntidadeVersaoArquivos { versaoTag, arquivos SEQUENCE OF ArquivoAssinatura } -> md::CVersoesArquivos.
// The list is copied into a std::map (__tree_balance_after_insert, key compare = memcmp), so duplicated names keep
// the first entry; the md constructor (func 5867) then rejects an empty tag or an empty list (8694 / 8695).
md::CVersoesArquivos CConversorVersoesArquivos::DoDesconverte(const TEntidade& entidade) const
{
    std::map<std::string, std::string> arquivos;
    for (const auto& arquivo : entidade.get_arquivos())
        arquivos.emplace(arquivo.get_nomeArquivo(), arquivo.get_assinatura());
    return md::CVersoesArquivos(entidade.get_versaoTag(), arquivos);                  // func 5867
}

}  // namespace comum::asn
