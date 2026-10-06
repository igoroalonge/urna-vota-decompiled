// Reconstructed from vota_web_wasm.wasm (unit u06). Original: uenux2/src/app/vota/eleitor/comum/ctelascargo.cpp
//
// srcloc evidence:
//   :60  std::string vota::NomeTela(const ETelaVotacao)                      "Tela inválida {}" (9327)
//   :67  vota::CTelasCargo::CTelasCargo(const comum::TCargoID, const TMapa &)   "não foi passada nenhuma tela" (9328)
//   :72  (same constructor)                                                   "tela nula [" ... "]" (9329)
//   :80  CFormInterativoTelaVota vota::CTelasCargo::GetTela(const ETelaVotacao) const   "Cargo {} não possui [{}]" (9330)
//   ctelasvota.cpp:3534  CFormInterativoTelaVota vota::CTelasVota::GetTelaCargo(const comum::TCargoID,
//                        const ETelaVotacao) const      "Nao existe o conjunto de telas associado ao cargo id {}" (9358)

#include "vota/eleitor/comum/ctelascargo.h"

#include <format>

#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

// wasm func 6718. Not observed executing (only the error paths below call it).
std::string NomeTela(const ETelaVotacao tela)
{
    switch (tela) {
    case ETelaVotacao::Inicial:                         return "Tela Inicial";
    case ETelaVotacao::Completa:                        return "Tela Completa";
    case ETelaVotacao::ConferenciaVotoNominal:          return "Tela de Conferência de Voto Nominal";
    case ETelaVotacao::VotoBranco:                      return "Tela de Voto Branco";
    case ETelaVotacao::ConferenciaVotoBranco:           return "Tela de Conferência de Voto Branco";
    case ETelaVotacao::VotoProporcionalNuloIncompleto:  return "Tela de Voto Proporcional Nulo Incompleto";
    case ETelaVotacao::VotoNulo:                        return "Tela de Voto Nulo";
    case ETelaVotacao::ConferenciaVotoNulo:             return "Tela de Conferência de Voto Nulo";
    case ETelaVotacao::Partido:                         return "Tela de Partido";
    case ETelaVotacao::VotoLegendaIncompleto:           return "Tela de Voto Legenda Incompleto";
    case ETelaVotacao::ConferenciaVotoLegenda:          return "Tela de Conferência de Voto Legenda";
    case ETelaVotacao::CandidatoInexistente:            return "Tela de Candidato Inexistente";
    case ETelaVotacao::ConferenciaCandidatoInexistente: return "Tela de Conferência de Candidato Inexistente";
    case ETelaVotacao::CandidatoInapto:                 return "Tela de Candidato Inapto";
    case ETelaVotacao::ConferenciaCandidatoInapto:      return "Tela de Conferência de Candidato Inapto";
    case ETelaVotacao::CandidatoRepetido:               return "Tela de Candidato Repetido";
    case ETelaVotacao::ConferenciaCandidatoRepetido:    return "Tela de Conferência de Candidato Repetido";
    case ETelaVotacao::CargoSemCandidato:               return "Tela de Cargo sem Candidato";
    }
    throw CUeVotaError(9327, std::format("Tela inválida {}", static_cast<int>(tela)),
                       std::source_location::current());                                   // :60
}

// wasm func 2386 — called from the start-up function 7787 (vota::CInformacaoEleitor::Inicializar), inside the
// inlined CTelasVota::CTelasVota. Observed executing.
CTelasCargo::CTelasCargo(const comum::TCargoID cargo, const TMapa& telas)
    : m_cargo(cargo)
    , m_telas(telas)            // map copy: insert(first, last) with hint, func 3251 = __tree::__find_equal
{
    const auto prefixo = std::format("Cargo {} ", cargo);                  // cargo formatted as a number
    if (m_telas.empty())
        throw CUeVotaError(9328, prefixo + "não foi passada nenhuma tela",
                           std::source_location::current());               // :67
    for (const auto& [tela, form] : m_telas)
        if (!form)
            throw CUeVotaError(9329, prefixo + "tela nula [" + NomeTela(tela) + "]",
                               std::source_location::current());           // :72
}

// srcloc :80 — inlined into func 4135 (below)
CFormInterativoTelaVota CTelasCargo::GetTela(const ETelaVotacao tela) const
{
    const auto it = m_telas.find(tela);
    if (it == m_telas.end())
        throw CUeVotaError(9330, std::format("Cargo {} não possui [{}]", m_cargo, NomeTela(tela)),
                           std::source_location::current());               // :80
    return it->second;
}

// wasm func 4135 (analyzer: CTelasCargo::GetTela). The function is really the CTelasVota method
// below (ctelasvota.cpp:3534, unit u07's file) with CTelasCargo::GetTela inlined. Observed executing.
CFormInterativoTelaVota CTelasVota::GetTelaCargo(const comum::TCargoID cargo, const ETelaVotacao tela) const
{
    const auto it = m_telasCargo.find(cargo);                            // std::map<TCargoID, CTelasCargo> at +0
    if (it == m_telasCargo.end())
        throw CUeVotaError(9358, std::format("Nao existe o conjunto de telas associado ao cargo id {}", cargo),
                           std::source_location::current());               // ctelasvota.cpp:3534
    return it->second.GetTela(tela);
}

// wasm func 3251: std::__tree<std::__value_type<ETelaVotacao, CFormInterativoTelaVota>, ...>::
//   __find_equal<ETelaVotacao>(const_iterator hint, __parent_pointer&, __node_base_pointer& dummy,
//   const ETelaVotacao&) — libc++ internals of the map copy above (identical-code-folded with other
//   uebyte-keyed maps: also called by api_f3266 and comum_f5681).

}  // namespace vota
