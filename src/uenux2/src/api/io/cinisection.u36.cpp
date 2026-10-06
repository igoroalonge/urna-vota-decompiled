// uenux2/src/api/io/cinisection.cpp (attested by the srcloc cinisection.cpp:36 of CIniSection::operator=)
//   --  FRAGMENT written by unit u36. Reconstructed from vota_web_wasm.wasm.
//
// api::CIniSection = { std::string m_nome (+0); std::map<std::string, CIniKey> m_chaves (+12) } - one
// "[section]" of an INI/.properties file (reader: api::CIniStrings, cinistrings.u12-fragment.cpp).
// CIniKey = { std::string nome (+0), std::string valor (+12) }, so in a map node the key is at +16 and
// the value string at +40.
#include "api/io/cinisection.h"

namespace api {

// wasm func 10877                                                                       // name inferred
// Out-of-line lookup of a key (std::map<std::string,...>::find = wasm 163). Its only caller is the merged
// body of comum::md::CVersoesContratos::GetValor / CDependenciasContratos::GetValor (wasm 6044), whose
// objects start with a CIniSection (the /etc/versoes.properties and /etc/dependencias.properties files,
// 0-byte stubs in the simulator). end() of the same map is the ICF body 5481 (`return this + 16`).
std::map<std::string, CIniKey>::const_iterator CIniSection::Find(const std::string& chave) const
{
    return m_chaves.find(chave);
}

}  // namespace api
