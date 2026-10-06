// FRAGMENT of uenux2/src/app/comum/dados/md/candidatura/cdadoscandidato.cpp (file attested by srclocs :35 and :65).
// Reconstructed from vota_web_wasm.wasm (unit u35). These constructors exist only inlined into
// comum::asn::CConversorDadosCandidato::DoDesconverte (wasm func 11450); no out-of-line copy survives.
//
// md::CDadosCandidato (52 bytes; one candidate or running mate):
//   +0  std::string m_codigo                      candidate code (links to the photo in -fo.dat)
//   +12 std::string m_nomeUrna                    name shown on the voting screen
//   +24 std::optional<std::string> m_nomeFonetico name for the speech synthesiser (flag +36)
//   +40 CSexo::ESexo m_sexo                       0 não informado, 1 masculino, 2 feminino
//   +44 ESituacao m_situacao                      0 apto, 1 inapto                        (names inferred)
//   +48 uebyte m_ordem                            0 = titular, 1..9 = ordem de suplência (vice / suplente)
#include <format>
#include <optional>
#include <string>

#include "comum/dados/dadosdefs.h"   // CDadosError = CBaseError<EUeComumDadosError> (typeinfo @1528076)
#include "comum/dados/md/eleitor/csexo.h"

namespace comum::md {

// titular without phonetic name (ordem 0, not validated)
CDadosCandidato::CDadosCandidato(std::string codigo, std::string nomeUrna, CSexo::ESexo sexo, ESituacao situacao)
    : m_codigo(std::move(codigo)), m_nomeUrna(std::move(nomeUrna)), m_sexo(sexo), m_situacao(situacao), m_ordem(0)
{
}

// titular with phonetic name (ordem 0, not validated)
CDadosCandidato::CDadosCandidato(std::string codigo, std::string nomeUrna, std::string nomeFonetico,
                                 CSexo::ESexo sexo, ESituacao situacao)
    : m_codigo(std::move(codigo)), m_nomeUrna(std::move(nomeUrna)), m_nomeFonetico(std::move(nomeFonetico)),
      m_sexo(sexo), m_situacao(situacao), m_ordem(0)
{
}

// srcloc cdadoscandidato.cpp:35 - CDadosCandidato(std::string, std::string, CSexo::ESexo, ESituacao, uebyte)
CDadosCandidato::CDadosCandidato(std::string codigo, std::string nomeUrna, CSexo::ESexo sexo, ESituacao situacao,
                                 uebyte ordem)
    : m_codigo(std::move(codigo)), m_nomeUrna(std::move(nomeUrna)), m_sexo(sexo), m_situacao(situacao), m_ordem(ordem)
{
    if (m_ordem < 1 || m_ordem > 9)                            // compiled as (uint8)(ordem - 10) <= 246
        throw CDadosError(EUeComumDadosError{7998}, std::format("Suplente com ordem inválida: {}", m_ordem));   // :35
}

// srcloc cdadoscandidato.cpp:65 - CDadosCandidato(std::string, std::string, std::string, CSexo::ESexo, ESituacao, uebyte)
CDadosCandidato::CDadosCandidato(std::string codigo, std::string nomeUrna, std::string nomeFonetico,
                                 CSexo::ESexo sexo, ESituacao situacao, uebyte ordem)
    : m_codigo(std::move(codigo)), m_nomeUrna(std::move(nomeUrna)), m_nomeFonetico(std::move(nomeFonetico)),
      m_sexo(sexo), m_situacao(situacao), m_ordem(ordem)
{
    if (m_ordem < 1 || m_ordem > 9)
        throw CDadosError(EUeComumDadosError{7999}, std::format("Suplente com ordem inválida: {}", m_ordem));   // :65
}

} // namespace comum::md
