// Reconstructed from vota_web_wasm.wasm (unit u06).
// Original: uenux2/src/app/vota/eleitor/comum/ctelascargo.h (path inferred)
//
// CTelasCargo = the set of pre-built voting screens (forms) of ONE cargo, indexed by ETelaVotacao.
// CTelasVota (unit u07) builds one CTelasCargo per cargo at start-up (inside func 7787) and keeps
// them in a std::map<TCargoID, CTelasCargo> at CTelasVota +0.
#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "api/gui/cinteractiveform.h"     // api::CInteractiveForm<api::IScreen, api::IInputKbd>
#include "comum/dados/ccargos.h"          // comum::TCargoID (uebyte)

namespace vota {

using uebyte = std::uint8_t;
// Same alias as in ctelasvota.h (unit u07, "attested name"): 8 bytes {ptr, ctrl}. An alias to a
// different type here would make the two headers ill-formed together (ctelasvota.h includes this one).
using CFormInterativoTelaVota = std::shared_ptr<api::CInteractiveForm<api::IScreen, api::IInputKbd>>;

/// Screens of the voting flow. Values and names from NomeTela (func 6718).
enum class ETelaVotacao : uebyte {
    Inicial = 0,                            // "Tela Inicial"
    Completa = 1,                           // "Tela Completa" (candidate data + CONFIRMA/CORRIGE)
    ConferenciaVotoNominal = 2,             // "Tela de Conferência de Voto Nominal"
    VotoBranco = 3,                         // "Tela de Voto Branco"
    ConferenciaVotoBranco = 4,              // "Tela de Conferência de Voto Branco"
    VotoProporcionalNuloIncompleto = 5,     // "Tela de Voto Proporcional Nulo Incompleto"
    VotoNulo = 6,                           // "Tela de Voto Nulo"
    ConferenciaVotoNulo = 7,                // "Tela de Conferência de Voto Nulo"
    Partido = 8,                            // "Tela de Partido"
    VotoLegendaIncompleto = 9,              // "Tela de Voto Legenda Incompleto"
    ConferenciaVotoLegenda = 10,            // "Tela de Conferência de Voto Legenda"
    CandidatoInexistente = 11,              // "Tela de Candidato Inexistente"
    ConferenciaCandidatoInexistente = 12,   // "Tela de Conferência de Candidato Inexistente"
    CandidatoInapto = 13,                   // "Tela de Candidato Inapto"
    ConferenciaCandidatoInapto = 14,        // "Tela de Conferência de Candidato Inapto"
    CandidatoRepetido = 15,                 // "Tela de Candidato Repetido"
    ConferenciaCandidatoRepetido = 16,      // "Tela de Conferência de Candidato Repetido"
    CargoSemCandidato = 17,                 // "Tela de Cargo sem Candidato"
};

/// Human-readable name of a screen (used in error messages). func 6718, srcloc ctelascargo.cpp:60
std::string NomeTela(const ETelaVotacao tela);

class CTelasCargo {
public:
    using TMapa = std::map<ETelaVotacao, CFormInterativoTelaVota>;

    CTelasCargo(const comum::TCargoID cargo, const TMapa& telas);   // func 2386 (srcloc 67, 72)

    CFormInterativoTelaVota GetTela(const ETelaVotacao tela) const;  // srcloc 80 (inlined into func 4135)

private:
    comum::TCargoID m_cargo;                // +0
    TMapa m_telas;                          // +4 (map: begin +4, root +8, size +12); size 16
};

}  // namespace vota
