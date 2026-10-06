// FRAGMENT of uenux2/src/app/comum/dados/md/processoeleitoral/cdetalheconsulta.cpp (attested by srclocs :29 / :32).
// Reconstructed from vota_web_wasm.wasm (unit u35). The constructor exists only inlined into
// comum::asn::CConversorDetalheConsulta::DoDesconverte (wasm func 11377).
//
// md::CDetalheConsulta (48 bytes): +0 nome, +12 pergunta ('|' = line break), +24 std::vector<CRespostaConsulta>
// (copied with comum_f3018 = vector range-construct), +36 textoFonetico.
#include <string>

#include "comum/dados/dadosdefs.h"   // CDadosError (EUeComumDadosError)

namespace comum::md {

// The nome is not validated.
CDetalheConsulta::CDetalheConsulta(const std::string& nome, const std::string& pergunta,
                                   const TVectorRespostaConsulta& respostas, const std::string& textoFonetico)
    : m_nome(nome), m_pergunta(pergunta), m_respostas(respostas), m_textoFonetico(textoFonetico)
{
    if (m_pergunta.empty())
        throw CDadosError(EUeComumDadosError{8146}, "Pergunta inválida.");              // line 29
    if (m_respostas.empty())
        throw CDadosError(EUeComumDadosError{8147}, "Conjunto de respostas vazio.");    // line 32
}

} // namespace comum::md
