// uenux2/src/app/comum/gravadores/md/cversoescontratos.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23). Both classes are "key=value" property files read at the
// encerramento by CGeracaoVersoesContratos (see cgravaresultado.cpp, unit u07):
//   CVersoesContratos     <- /etc/versoes.properties       ("Modulo=versão", plus "tag=...")
//   CDependenciasContratos<- /etc/dependencias.properties  ("Modulo=Dep1,Dep2,...", plus "tag=...")
// In the web simulator both files are 0-byte stubs, so any lookup throws "Propriedade inexistente".
#include <format>
#include <map>
#include <string>

#include "comum/gravadores/iresultado.h"

namespace comum::md {

// wasm func 3825 (srcloc cversoescontratos.cpp:42). In the binary both GetValor are 20-byte thunks into one
// merged body (wasm func 6044, "merge-similar-functions") that takes the srcloc and the error code as parameters.
const std::string CVersoesContratos::GetValor(const std::string& chave)
{
    const auto it = m_propriedades.find(chave);                                          // comum_f10877
    if (it == m_propriedades.end())
        throw CUeComumGravadoresError(8697, std::format("Propriedade inexistente: {}", chave));   // :42
    return it->second;
}

}  // namespace comum::md
