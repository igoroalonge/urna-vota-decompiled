// uenux2/src/app/comum/dados/md/rdv/cvoto.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :80
// (constructor). EhNulo (funcs 5645, 11316) is in cvoto.u13.cpp.
//
// comum::md::CVoto (16 bytes) = one vote of the RDV ("registro digital do voto"):
//   +0 ETipo m_tipo     +4 std::string m_digitado (the digits typed, "" when none)
// ETipo follows the ASN.1 ModuloRegistroDigitalVoto TipoVoto.
//
// Errors: CRdvError = CBaseError<api::EUeRdvError, SErrorLimits{4650, 4850}> (typeinfo @1559972,
// constructor thunk func 655).
#include "comum/dados/md/rdv/cvoto.h"

#include <string>

#include "comum/dados/rdvdefs.h"   // CRdvError

namespace comum::md {

// wasm func 2256 (srcloc :80) - observed executing (every confirmed vote, and the RDV decoder
// CConversorVoto::DesconverteTipo). Enforces which vote types carry digits:
//   legenda (1), nominal (2), nulo por repetição (7)      -> digits REQUIRED
//   nulo (4)                                              -> anything (the invalid number typed)
//   branco (3), branco/nulo após suspensão (5, 6),
//   nulo em cargo sem candidato (8, 9)                    -> digits FORBIDDEN
CVoto::CVoto(ETipo tipo, const std::string& digitado)
    : m_tipo(tipo), m_digitado(digitado)                                       // func 1374 = string copy
{
    std::string erro;
    const auto exigeDigitado = [&](const std::string& nome) {
        if (m_digitado.empty())
            erro = "CVoto - voto " + nome + " com digitado vazio";
    };
    const auto proibeDigitado = [&](const std::string& nome) {
        if (!m_digitado.empty())
            erro = "CVoto - voto " + nome + " com digitado nao vazio";
    };

    switch (m_tipo) {
    case ETipo::LEGENDA:                             exigeDigitado("de legenda"); break;
    case ETipo::NOMINAL:                             exigeDigitado("nominal"); break;
    case ETipo::BRANCO:                              proibeDigitado("em branco"); break;
    case ETipo::NULO:                                break;
    case ETipo::BRANCO_APOS_SUSPENSAO:               proibeDigitado("suspenso branco"); break;
    case ETipo::NULO_APOS_SUSPENSAO:                 proibeDigitado("suspenso nulo"); break;
    case ETipo::NULO_POR_REPETICAO:                  exigeDigitado("repetido nulo"); break;
    case ETipo::NULO_CARGO_SEM_CANDIDATO:            proibeDigitado("em cargo sem candidato"); break;
    case ETipo::NULO_APOS_SUSPENSAO_CARGO_SEM_CANDIDATO: proibeDigitado("suspenso em cargo sem candidato"); break;
    default:
        erro = "CVoto - tipo de voto invalido " + std::to_string(static_cast<int>(m_tipo));   // func 296
        break;
    }
    if (!erro.empty())
        throw CRdvError(api::EUeRdvError{4661}, erro);                         // :80
}

} // namespace comum::md
