// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/estadoaplicacao/clocalidadeeleitoral.cpp
#include "clocalidadeeleitoral.h"

namespace comum::md::estadoaplicacao {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 5631: constructor with ValidaCriacao inlined (srclocs lines 18, 21, 24). Callers:
// comum::asn::CConversorLocalidadeEleitoral::DoDesconverte (11389) and a simulator mock (mock_f9863).
CLocalidadeEleitoral::CLocalidadeEleitoral(std::uint32_t municipio, std::uint16_t zona, std::uint16_t secao)
    : m_municipio(municipio), m_zona(zona), m_secao(secao)
{
    ValidaCriacao();
}

void CLocalidadeEleitoral::ValidaCriacao() const
{
    if (m_municipio >= 100000)
        throw CUeComumDadosError(8095, "Município inválido.");   // line 18
    if (m_zona >= 10000)
        throw CUeComumDadosError(8096, "Zona inválida.");        // line 21
    if (m_secao >= 10000)
        throw CUeComumDadosError(8097, "Seção inválida.");       // line 24
}

}  // namespace comum::md::estadoaplicacao
