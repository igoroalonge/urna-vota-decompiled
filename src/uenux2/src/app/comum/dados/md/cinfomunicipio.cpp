// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/cinfomunicipio.cpp
//
// CInfoMunicipio: municipality code, name, time-zone offset in minutes (fuso, -720..720) and an
// optional daylight-saving period (CHorarioVerao). Layout: +0 codigo, +4 nome, +16 int16 fuso,
// +20 optional<CHorarioVerao> (engaged +40).
#include "cinfomunicipio.h"

#include <format>

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 3725 (srcloc line 51)
const CHorarioVerao& CInfoMunicipio::GetHorarioVerao() const
{
    if (!m_horarioVerao.has_value())
        throw CUeComumDadosError(8008, "Não há informação de horário de verão.");
    return *m_horarioVerao;
}

// wasm func 5675 (srclocs lines 61, 66, 69). Called by the two constructors (comum_f5674 / 5676).
void CInfoMunicipio::ValidaCriacao() const
{
    if (m_codigo >= 100000)
        throw CUeComumDadosError(8009, std::format("Código de município inválido: {}", m_codigo));  // 61
    if (m_nome.empty())
        throw CUeComumDadosError(8010, "Nome vazio.");                                               // 66
    if (m_fuso < -720 || m_fuso > 720)                    // compiled as (uint16)(fuso - 721) <= 64094
        throw CUeComumDadosError(8011, std::format("Fuso-horário inválido: {}", m_fuso));           // 69
}

}  // namespace comum::md
