// ecourna-lib/ecourna/app/dados/cidentificacaosecaoeleitoral.h   (path inferred; also declares CMunicipioZona,
//   whose own header name - probably cmunicipiozona.h - is unknown)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// = ModuloTiposEleitorais::IdentificacaoSecaoEleitoral { municipioZona MunicipioZona { municipio, zona },
//   local INTEGER (1..9999), secao INTEGER (1..9999) }: which polling section ("seção eleitoral") a file refers
//   to. Used by the attendance result (ComparecimentoSecao) written at the end of voting.
#pragma once

#include "ecourna/api/pattern/cbasetype.hpp"

namespace ecourna::app::dados {

using api::pattern::CBaseType;
// (u11's cconversoridentificacaosecaoeleitoral.h calls the last two TNumeroLocal / TNumeroSecao.)
using TMunicipioID = CBaseType<unsigned int, 0, 99999, 6>;    // "MunicipioID" (func 2851)      alias inferred
using TZonaID = CBaseType<unsigned short, 0, 9999, 7>;        // "ZonaID"      (func 5850)      alias inferred
using TLocalID = CBaseType<unsigned int, 0, 9999, 8>;         // "LocalID"     (func 5849)      alias inferred
using TSecaoID = CBaseType<unsigned short, 0, 9999, 9>;       // "SecaoID"     (func 5848)      alias inferred

class CMunicipioZona {                                   // 8 bytes
public:
    // wasm func 5108 (not in u40). The two CBaseType arguments arrive as POINTERS (5108 loads them), so they are
    // references: a by-value single-member class would be passed as a plain i32 by the wasm C ABI (as the
    // TLocalID/TSecaoID arguments of 5110 are).
    CMunicipioZona(const TMunicipioID& municipio, const TZonaID& zona);
    TMunicipioID GetMunicipio() const { return m_municipio; }
    TZonaID GetZona() const { return m_zona; }
private:
    TMunicipioID m_municipio;   // +0
    TZonaID m_zona;             // +4
};

class CIdentificacaoSecaoEleitoral {                     // 16 bytes, trivially copyable
public:
    // wasm func 5110 (callers: CConversorIdentificacaoSecaoEleitoral::DoDeconverte 9127, CGravadorRCSecao 11616)
    CIdentificacaoSecaoEleitoral(const CMunicipioZona& municipioZona, TLocalID local, TSecaoID secao);

    const CMunicipioZona& GetMunicipioZona() const { return m_municipioZona; }
    TLocalID GetLocal() const { return m_local; }
    TSecaoID GetSecao() const { return m_secao; }

private:
    CMunicipioZona m_municipioZona;   // +0
    TLocalID m_local;                 // +8
    TSecaoID m_secao;                 // +12
};

} // namespace ecourna::app::dados
