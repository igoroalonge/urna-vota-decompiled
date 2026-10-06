// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/iregraidentidade.cpp
#include "iregraidentidade.h"

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;

// wasm func 5159  name inferred: copy of s without its leading '0' characters ("" if all zeros)
std::string SemZerosEsquerda(const std::string& s)
{
    std::string r = s;
    const auto pos = r.find_first_not_of('0');
    if (pos == std::string::npos) r.clear(); else r.erase(0, pos);
    return r;
}
}  // namespace

// wasm func 3723  name inferred (tools: vota_f3723; callers vota::CValidaIdentidade::vf2, vota_f2803)
bool IRegraIdentidade::EhValida(const std::string& identidade) const
{
    if (identidade.empty() || identidade.find_first_not_of("0123456789") != std::string::npos)
        return false;
    return Verifica(SemZerosEsquerda(identidade));
}

// (no function of its own: inlined into func 566)  (srcloc line 23)
void IRegraIdentidade::Valida(const std::string& identidade) const
{
    if (!EhValida(identidade))
        throw CUeComumDadosError(8109, "Identidade inválida: " + identidade);
}

}  // namespace comum::md
