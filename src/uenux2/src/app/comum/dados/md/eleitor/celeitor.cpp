// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/eleitor/celeitor.cpp
#include "celeitor.h"

#include "ecourna/api/util/cstringutils.h"

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
using ecourna::api::util::CStringUtils;   // Trim = comum_f1374 -> ecourna_f9406 (whitespace <= ' ')
}

// wasm func 5667 (srclocs lines 103, 107, 113). Called from the two CEleitor constructors
// (comum_f5666 / comum_f5668, not in this unit).
void CEleitor::ValidaCriacao() const
{
    if (CStringUtils::Trim(m_nome).empty())
        throw CUeComumDadosError(8064, "Nome vazio");                    // line 103
    if (m_necessidadeEspecial >= 2)
        throw CUeComumDadosError(8065, "Necessidade especial inválida"); // line 107
    if (!m_nomeSocial.empty() && CStringUtils::Trim(m_nomeSocial).empty())
        throw CUeComumDadosError(8066, "Nome social vazio");             // line 113
}

}  // namespace comum::md
