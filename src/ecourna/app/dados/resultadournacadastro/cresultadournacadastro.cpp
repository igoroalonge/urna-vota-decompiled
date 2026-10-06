// ecourna-lib/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14 (the two getters; constructors are in unit u40).
#include "ecourna/app/dados/resultadournacadastro/cresultadournacadastro.h"

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 9015 (srcloc line 46)
const CDadosComparecimento& CResultadoUrnaCadastro::GetDadosComparecimento() const
{
    if (!m_dadosComparecimento.has_value()) {
        throw CDadosResultadoUrnaCadastroError(3276, "Dados de comparecimento não definidos para este objeto.");            // line 46
    }
    return *m_dadosComparecimento;
}

// wasm func 9014 (srcloc line 57)
const CDadosComparecimentoCifrado& CResultadoUrnaCadastro::GetDadosComparecimentoCifrado() const
{
    if (!m_dadosComparecimentoCifrado.has_value()) {
        throw CDadosResultadoUrnaCadastroError(3277, "Dados de comparecimento cifrados não definidos para este objeto.");   // line 57
    }
    return *m_dadosComparecimentoCifrado;
}

} // namespace ecourna::app::dados
