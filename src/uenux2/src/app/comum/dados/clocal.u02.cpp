// uenux2/src/app/comum/dados/clocal.cpp  --  FRAGMENT written by unit u02 (file owned by u04)
//
// comum::CLocal holds the polling-place file (ModuloLocal::Local: UF, município, zona, seção,
// biometric flag...). Its accessors check that the file was read (CLocal::VerificaLido, wasm 782,
// receives the name of the calling method as a std::string for the error message) and then read
// a field of the decoded record. Names come from those strings.
//
// Layout: CLocal (4 bytes) = { std::unique_ptr<md::CLocal> m_dados; }  (+0)
//   md::CLocal (see u04): +16 std::string uf, +40 TMunicipioID municipio, +44 std::string
//   nomeMunicipio, +58 bool urnaBiometrica, optional<CSecaoEleitoral> at +84 (engaged flag +120),
//   optional<CIdentificacaoUrnaContingencia> at +124 (engaged flag +136).

#include "comum/dados/clocal.h"

namespace comum {

// wasm func 401: lazy singleton (object @1838900, mutex residue @1838876)          // name inferred
CLocal& CLocal::GetInst()
{
    static std::unique_ptr<CLocal> ms_instancia;
    if (!ms_instancia)
        ms_instancia.reset(new CLocal());      // wasm 2262; the replaced object is destroyed by wasm 2261
    return *ms_instancia;
}

// wasm func 2262: `m_dados = nullptr; return this;` (also used by other 4-byte holders)
CLocal::CLocal() : m_dados(nullptr) {}

// wasm func 1004
TMunicipioID CLocal::GetMunicipio() const
{
    VerificaLido("GetMunicipio");
    return m_dados->municipio;                  // +40
}

// wasm func 1077
const std::string& CLocal::GetNomeMunicipio() const
{
    VerificaLido("GetNomeMunicipio");
    return m_dados->nomeMunicipio;              // +44
}

// wasm func 1702
const std::string& CLocal::GetUF() const
{
    VerificaLido("GetUF");
    return m_dados->uf;                         // +16
}

// wasm func 5743
const md::CLocal& CLocal::GetLocalData() const
{
    VerificaLido("GetLocalData");
    return *m_dados;
}

// wasm func 820
bool CLocal::UrnaBiometrica() const
{
    VerificaLido("UrnaBiometrica");
    // byte +120 is the engaged flag of the optional seção (not a "read" flag: VerificaLido already
    // checked that): a contingency urna (no seção) is never biometric
    return m_dados->secao.has_value() && m_dados->urnaBiometrica;    // bytes +120 and +58
}

} // namespace comum
