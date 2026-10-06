// uenux2/src/app/comum/gravadores/md/cdependenciascontratos.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//   CDependenciasContratos <- /etc/dependencias.properties ("Modulo=Dep1,Dep2,...", plus "tag=...").
#include <format>
#include <string>

#include "comum/gravadores/iresultado.h"
#include "comum/gravadores/md/cdependenciascontratos.h"

namespace comum::md {

// wasm func 3826 (srcloc cdependenciascontratos.cpp:59). In the binary both GetValor are 20-byte thunks into one
// merged body (wasm func 6044, "merge-similar-functions") that takes the srcloc and the error code as parameters.
const std::string CDependenciasContratos::GetValor(const std::string& chave)
{
    const auto it = m_propriedades.find(chave);                                          // comum_f10877
    if (it == m_propriedades.end())
        throw CUeComumGravadoresError(8663, std::format("Propriedade inexistente: {}", chave));   // :59
    return it->second;
}

}  // namespace comum::md
