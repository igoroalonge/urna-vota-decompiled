// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/municipiozona/cmunicipio.cpp
#include "cmunicipio.h"

#include "ecourna/api/util/cstringutils.h"

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 3712 (srcloc line 27). Built by comum::asn::CConversorMunicipio::DoDesconverte (11378).
CMunicipio::CMunicipio(TMunicipioID codigo, const std::string& nome, bool comBiometria)
    : m_codigo(codigo)
    , m_nome(ecourna::api::util::CStringUtils::Trim(nome))   // comum_f1374
    , m_comBiometria(comBiometria)
{
    if (m_nome.empty())
        throw CUeComumDadosError(8119, "Nome vazio");
}

}  // namespace comum::md
