// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.cpp
#include "ccomplementomunicipio.h"

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 2795 (srcloc line 37)
const CHorarioVerao& CComplementoMunicipio::GetHorarioVerao() const
{
    if (!m_horarioVerao.has_value())
        throw CUeComumDadosError(8112, "Não há informação de horário de verão.");
    return *m_horarioVerao;
}

// wasm func 5647 (srclocs lines 47, 52)
void CComplementoMunicipio::ValidaCriacao() const
{
    if (m_codigoMunicipio >= 100000)
        throw CUeComumDadosError(8113, "Código de município inválido.");   // line 47
    if (m_fuso < -720 || m_fuso > 720)                  // compiled as (uint16)(fuso - 721) <= 64094
        throw CUeComumDadosError(8114, "Fuso-horário inválido.");          // line 52
}

}  // namespace comum::md
