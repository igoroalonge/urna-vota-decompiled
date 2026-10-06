// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/chorarioveraomunicipio.cpp
// CHorarioVeraoMunicipio: daylight-saving period of one municipality (+0 codigo do município).
#include "chorarioveraomunicipio.h"

#include <format>

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 5678 (srcloc line 47)
void CHorarioVeraoMunicipio::ValidaCriacao() const
{
    if (m_codigoMunicipio >= 100000)
        throw CUeComumDadosError(8006, std::format("Código de município inválido: {}", m_codigoMunicipio));
}

}  // namespace comum::md
