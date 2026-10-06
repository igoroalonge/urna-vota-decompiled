// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/cvalidadoridentidade.cpp
#include "cvalidadoridentidade.h"

#include <algorithm>
#include <format>

#include "api/pattern/csingleton.h"

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 1926 (srcloc line 26)
CValidadorIdentidade& CValidadorIdentidade::GetInst()
{
    return api::CSingleton<CValidadorIdentidade>::GetInst("CValidadorIdentidade - instancia nao criada");
}

// (no function of its own: inlined into CEleitorIdentidade::CEleitorIdentidade, func 566) (srcloc line 61)
void CValidadorIdentidade::Valida(ETipoIdentificadorEleitor tipo, const std::string& identidade) const
{
    // The first rule that is either the "free identifier" rule or the rule of the requested type
    // wins. If a CRegraIdentidadeLivre is registered before the título/CPF rules, it accepts any
    // digit string for every type (see u05 doc).
    const auto regra = std::find_if(m_regras.begin(), m_regras.end(), [tipo](const auto& r) {
        return r->GetTipo() == ETipoIdentificadorEleitor::LIVRE || r->GetTipo() == tipo;
    });
    if (regra == m_regras.end())
        throw CUeComumDadosError(8108, std::format("Tipo de identidade inválido: {}", static_cast<int>(tipo)));
    (*regra)->Valida(identidade);   // IRegraIdentidade::Valida, iregraidentidade.cpp:23 (inlined)
}

}  // namespace comum::md
