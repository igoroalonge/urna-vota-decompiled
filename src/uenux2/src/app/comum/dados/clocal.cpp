// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/clocal.cpp
//
// See ccargos.cpp for the error types.
#include "clocal.h"

#include <format>

namespace comum {

using CErroDados = ecourna::api::exception::CBaseError<EUeComumDadosError>;

// wasm func 782 (srcloc line 345)
void CLocal::VerificaLido(const std::string& funcao) const
{
    if (!m_local)
        throw CErroDados(EUeComumDadosError{7874},
                         std::format("{}: o arquivo de locais ainda não foi carregado", funcao));
}

// wasm func 1003 (srcloc line 86). Note the string passed is "GetZona", not "GetZonaID".
TZonaID CLocal::GetZonaID() const
{
    VerificaLido("GetZona");
    if (m_local->TemSecao())                                   // flag +120
        return m_local->GetSecao().GetZona();                  // md::CLocal::GetSecao (943) +12
    if (m_local->TemContingencia())                            // flag +136
        return m_local->GetContingencia().GetZona();           // md::CLocal::GetContingencia (3724) +8
    throw CErroDados(EUeComumDadosError{7872}, "Tipo de local inválido");
}

// wasm func 1078 (srcloc line 110). A contingency urna has no section: returns 0.
TSecaoID CLocal::GetSecaoID() const
{
    VerificaLido("GetSecaoID");
    if (m_local->TemSecao())
        return m_local->GetSecao().GetSecao();                 // +20
    if (m_local->TemContingencia())
        return 0;
    throw CErroDados(EUeComumDadosError{7873}, "Tipo de local inválido");
}

// wasm func 5742 (srcloc line 354). Callers: comum_f1933 (seções agregadas, BU/QR "AGRE") and
// CRelatorioTesteImpressora.
void CLocal::VerificaEhSecao(const std::string& funcao) const
{
    VerificaLido(funcao);
    if (!m_local->TemSecao())
        throw CErroDados(EUeComumDadosError{7875}, std::format("{}: o local não era de seção", funcao));
}

}  // namespace comum
