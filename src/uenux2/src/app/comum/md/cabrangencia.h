// uenux2/src/app/comum/md/cabrangencia.h
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// md::CAbrangencia = "abrangência" (geographic scope) of an election/party/candidate datum:
// municipal (UF + município), estadual (UF only) or federal (whole country, no UF).
// C++ model of ModuloTiposEleitorais::Abrangencia { tipo TipoAbrangencia, id IdentificacaoAbrangencia OPTIONAL
// { siglaUF, codigoMunicipio OPTIONAL } }; converter = comum::asn::CConversorAbrangencia (unit u21).
// Not polymorphic. sizeof 20.
#pragma once

#include <string>

#include "comum/comumtypes.h"          // TMunicipioID   (header name ?)
#include "ecourna/api/exception/cbaseerror.hpp"

namespace comum::md {

// Same numbers as ASN.1 TipoAbrangencia (municipal 0, estadual 1, federal 2).
enum class ETipoAbrangencia : int { Municipal = 0, Estadual = 1, Federal = 2 };

// Error family of comum/md: CBaseError<EUeComumMdError, SErrorLimits{8900, 8950}> (typeinfo @1552724,
// vtable @1552744; constructor thunk = comum_f591). Codes used in this unit's files:
//   8900..8905 CAbrangencia, 8906..8907 CCabecalhoEntidade, 8916..8917 CIdentificacaoSecao,
//   8918..8920 CIdentificacaoUrna, 8925..8927 CSeguranca  (8915: see cidentificacaosecao.cpp).
enum class EUeComumMdError : int {};
using CUeComumMdError =
    ecourna::api::exception::CBaseError<EUeComumMdError, ecourna::api::exception::SErrorLimits{8900, 8950}>;

class CAbrangencia {
public:
    CAbrangencia(ETipoAbrangencia tipo, const std::string& uf, TMunicipioID municipio);   // wasm 3739

    ETipoAbrangencia GetTipo() const { return m_tipo; }
    const std::string& GetUF() const { return m_uf; }
    TMunicipioID GetMunicipio() const { return m_municipio; }

private:
    ETipoAbrangencia m_tipo;        // +0
    std::string      m_uf;          // +4   "" (federal) or 2 letters
    TMunicipioID     m_municipio;   // +16  0 unless municipal
};

} // namespace comum::md
