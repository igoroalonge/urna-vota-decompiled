// FRAGMENT of uenux2/src/app/comum/dados/md/processoeleitoral/crespostaconsulta.cpp (attested by srclocs :29 / :32).
// Reconstructed from vota_web_wasm.wasm (unit u35). The constructor exists only inlined into
// comum::asn::CConversorDetalheConsulta::DoDesconverte (wasm func 11377).
//
// md::CRespostaConsulta (28 bytes; one possible answer of a consulta, typed by the voter as a number):
//   +0 TCandidatoID m_numero   +4 std::string m_resposta   +16 std::string m_textoFonetico
#include <string>

#include "comum/dados/dadosdefs.h"   // CDadosError (EUeComumDadosError, typeinfo @1528076)

namespace comum::md {

CRespostaConsulta::CRespostaConsulta(TCandidatoID numero, const std::string& resposta, const std::string& textoFonetico)
    : m_numero(numero), m_resposta(resposta), m_textoFonetico(textoFonetico)
{
    if (m_numero >= 100000)
        throw CDadosError(EUeComumDadosError{8167}, "Número de votável inválido.");     // line 29
    if (m_resposta.empty())
        throw CDadosError(EUeComumDadosError{8168}, "Resposta inválida.");              // line 32
}

} // namespace comum::md
