// uenux2/src/app/comum/gravadores/md/cversoesarquivos.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// md::CVersoesArquivos = ModuloVersaoArquivos::EntidadeVersaoArquivos (mr.ver): the "tag" of the ASN.1 contracts
// and the version of every ASN.1 module used by the result files.
//   +0 std::string m_versaoTag     +12 std::map<std::string, std::string> m_arquivos (module/file -> version)
#include <map>
#include <string>

#include "comum/gravadores/iresultado.h"
#include "comum/gravadores/md/cversoesarquivos.h"   // (header written by unit u21)
//   class CVersoesArquivos { std::string m_versaoTag (+0); std::map<std::string, std::string> m_arquivos (+12); };

namespace comum::md {

// wasm func 5867. The tools used the name of the inlined ValidaCriacao (srcloc :28, :33); the function
// is the constructor: it copies the tag and inserts every (module, version) pair (comum_f1221 = map insert),
// then validates. Callers: CGravaResultado::StartState (12098) and CConversorVersoesArquivos::DoDesconverte (10263).
CVersoesArquivos::CVersoesArquivos(const std::string& tag, const std::map<std::string, std::string>& versoes)
    : m_versaoTag(tag), m_arquivos(versoes.begin(), versoes.end())
{
    ValidaCriacao();
}

void CVersoesArquivos::ValidaCriacao() const
{
    if (m_versaoTag.empty())
        throw CUeComumGravadoresError(8694, "Versão da TAG inválida");           // :28
    if (m_arquivos.empty())
        throw CUeComumGravadoresError(8695, "Versões de arquivos inválida");     // :33
}

}  // namespace comum::md
