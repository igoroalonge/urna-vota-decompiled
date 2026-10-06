// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/clocal.cpp
#include "clocal.h"

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 943 (srcloc line 59)
const CSecaoEleitoral& CLocal::GetSecao() const
{
    if (!m_secao.has_value())
        throw CUeComumDadosError(8012, "Não há informação.");
    return *m_secao;
}

// wasm func 3724 (srcloc line 67). Body = shared optional getter comum_f6029(this, 124, srcloc,
// "Não há informação.", 8013) (wasm-opt merged it with CCargo::GetDetalheConsulta; the annotation
// "Falha ao validar assinatura UE..." in the pseudo-code is a false string hit on the constant 8013).
const CIdentificacaoUrnaContingencia& CLocal::GetContingencia() const
{
    if (!m_contingencia.has_value())
        throw CUeComumDadosError(8013, "Não há informação.");
    return *m_contingencia;
}

// wasm func 5673 (srclocs lines 75, 78, 81, 84, 89). Called by comum::asn::CConversorLocal::DoDesconverte.
void CLocal::ValidaCriacao() const
{
    if (m_pais.empty())
        throw CUeComumDadosError(8014, "País inválido.");                                            // 75
    if (m_siglaUF.size() != 2)
        throw CUeComumDadosError(8015, "Sigla da UF inválida.");                                     // 78
    if (m_nomeUF.empty())
        throw CUeComumDadosError(8016, "Nome da UF inválido.");                                      // 81
    if (m_secao && m_secao->GetMunicipio() != m_municipio.GetCodigo())
        throw CUeComumDadosError(8017, "Códigos de município incompatíveis para seção.");            // 84
    if (m_contingencia && m_contingencia->GetMunicipio() != m_municipio.GetCodigo())
        throw CUeComumDadosError(8018, "Códigos de município incompatíveis para urna de contingência."); // 89
}

}  // namespace comum::md
