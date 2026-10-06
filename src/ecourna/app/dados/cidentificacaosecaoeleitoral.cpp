// ecourna-lib/ecourna/app/dados/cidentificacaosecaoeleitoral.cpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
#include "ecourna/app/dados/cidentificacaosecaoeleitoral.h"

namespace ecourna::app::dados {

// wasm func 5110 - plain copies (i64 for CMunicipioZona, i32 local, i16 seção); the range checks happened when
// the CBaseType arguments were built by the caller.
CIdentificacaoSecaoEleitoral::CIdentificacaoSecaoEleitoral(const CMunicipioZona& municipioZona, TLocalID local,
                                                           TSecaoID secao)
    : m_municipioZona(municipioZona)
    , m_local(local)
    , m_secao(secao)
{
}

} // namespace ecourna::app::dados
