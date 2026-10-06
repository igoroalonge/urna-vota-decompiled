// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/estadoaplicacao/cajustedatahora.cpp
//
// comum::md::estadoaplicacao::CAjusteDataHora = the pending clock adjustment stored in the urna's
// general state (ModuloEstadoGeralUrna::AjusteDataHora {tipoAjusteDataHora, valorAjusteDataHora}).
// Layout: +0 ETipoAjusteDataHora tipo, +4 ueint32 valor (seconds).
// (The api::CAjusteDataHora class that shares this file name in the analysis DB is a different
//  class; it is reconstructed in src/uenux2/src/api/util/cajustedatahora.cpp, path inferred.)
#include "cajustedatahora.h"

#include <format>

namespace comum::md::estadoaplicacao {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 3702 (srcloc line 32). Callers: vota::CAjusteInicial::ValidaTemposDesligamento,
// vota::CIniciodeCiclo::AjustaDataHora, vota::CEncerramentoHorarioInvalido::StartState.
void CAjusteDataHora::AdicionaDeltaT(const ueint32 deltaT)
{
    if (m_tipo != ETipoAjusteDataHora::eAlterarDataSistema)   // value 2
        // format arg = the enum (type 15 __handle, enum formatter func 536 prints the integer)
        throw CUeComumDadosError(8071, std::format("DeltaT pode ser adicionado apenas para eAlterarDataSistema {}",
                                                   m_tipo));
    m_valor += deltaT;
}

}  // namespace comum::md::estadoaplicacao
