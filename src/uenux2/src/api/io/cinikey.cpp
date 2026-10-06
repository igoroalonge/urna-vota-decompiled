// uenux2/src/api/io/cinikey.cpp
//
// Reconstructed from vota_web_wasm.wasm. Unit u18.
//
// api::CIniKey = one "key = value" line of an INI/.properties file read by api::CIniStrings
// (cinistrings.u12-fragment.cpp). 24 bytes: +0 std::string m_nome, +12 std::string m_valor.
// VOTA only uses it for /etc/dependencias.properties and /etc/versoes.properties (0-byte stubs in the
// simulator), read by comum::CGeracaoVersoesContratos when the result files are written.
#include "api/io/cinikey.h"

#include "api/io/euioerror.h"

namespace api {

// wasm func 5483 (srcloc line 24). Callers: the INI parser (5480, 5482) when a key appears again in a
// section: the value is replaced, the name must not change.
CIniKey& CIniKey::operator=(const CIniKey& outra)
{
    if (m_nome != outra.m_nome)
        throw CUeIoError(EUeIoError(6005), "O nome de uma chave não pode ser modificado.");          // line 24
    if (this != &outra)
        m_valor = outra.m_valor;
    return *this;
}

} // namespace api
