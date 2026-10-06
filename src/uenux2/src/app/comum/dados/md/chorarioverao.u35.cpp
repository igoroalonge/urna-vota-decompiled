// FRAGMENT of uenux2/src/app/comum/dados/md/chorarioverao.cpp (file attested by srclocs :29 and :32).
// Reconstructed from vota_web_wasm.wasm (unit u35). The constructor exists only inlined into
// comum::asn::CConversorHorarioVerao::DoDesconverte (wasm func 11384).
//
// md::CHorarioVerao (20 bytes): +0 api::CDate inicio, +8 api::CDate fim, +16 int diferenca (minutes).
#include "api/util/cdate.h"
#include "comum/dados/dadosdefs.h"   // CDadosError (EUeComumDadosError)

namespace comum::md {

CHorarioVerao::CHorarioVerao(const api::CDate& inicio, const api::CDate& fim, int diferenca)
    : m_inicio(inicio), m_fim(fim), m_diferenca(diferenca)
{
    if (!(m_fim > m_inicio))                                     // unknown_f1261 = CDate three-way comparison
        throw CDadosError(EUeComumDadosError{8003}, "Período inválido.");            // line 29
    if (m_diferenca == 0)
        throw CDadosError(EUeComumDadosError{8004}, "Diferença de horário nula.");   // line 32
}

} // namespace comum::md
