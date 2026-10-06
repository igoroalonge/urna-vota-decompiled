// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/eleitor/celeitordinamico.h
//
// CEleitorDinamico = the part of a voter's record that changes on election day (row of the SQLite
// table eleitor_dinamico, see dao/celeitordinamicodao.cpp). sizeof == 84.
#pragma once

#include <cstdint>
#include <optional>
#include <utility>

#include "celeitoridentidade.h"

namespace comum::md {

class CEleitorDinamico {
public:
    using TApresentacaoFoto = std::pair<int /*EEstadoApresentacaoFoto*/, int /*EResultadoApresentacaoFoto*/>;

    // wasm func 5663: with título of the poll worker who enabled the voter manually
    CEleitorDinamico(CEleitorIdentidade titulo, CEleitorIdentidade identidadeHabilitacao,
                     int estadoComparecimento, int tipoHabilitacao, int dedoHabilitacao,
                     std::uint16_t scoreHabilitacao, std::uint8_t numeroTentativa,
                     int tipoAtivacaoAudio, int erroDecifrarBiometria,
                     CEleitorIdentidade tituloMesarioHabilitacao, TApresentacaoFoto apresentacaoFoto);
    // wasm func 5664: without it
    CEleitorDinamico(CEleitorIdentidade titulo, CEleitorIdentidade identidadeHabilitacao,
                     int estadoComparecimento, int tipoHabilitacao, int dedoHabilitacao,
                     std::uint16_t scoreHabilitacao, std::uint8_t numeroTentativa,
                     int tipoAtivacaoAudio, int erroDecifrarBiometria, TApresentacaoFoto apresentacaoFoto);

    const CEleitorIdentidade& GetTituloMesarioHabilitacao() const;   // celeitordinamico.cpp:93
    bool HabilitadoManualmente() const { return m_tituloMesario.has_value(); }   // name inferred
    // (other getters are trivial and inlined everywhere)

private:
    CEleitorIdentidade m_titulo;                 // +0   (string +0, tipo +12)
    CEleitorIdentidade m_identidadeHabilitacao;  // +16  (document shown to enable the voter)
    int m_estadoComparecimento;                  // +32
    int m_tipoHabilitacao;                       // +36  (biometria / documento / mesário)
    int m_dedoHabilitacao;                       // +40  (CDedo::TipoDedo)
    std::uint16_t m_scoreHabilitacao;            // +44
    std::uint8_t m_numeroTentativa;              // +46  (0..4)
    int m_tipoAtivacaoAudio;                     // +48  (0..2)
    int m_erroDecifrarBiometria;                 // +52
    std::optional<CEleitorIdentidade> m_tituloMesario;   // +56, engaged flag +72
    TApresentacaoFoto m_apresentacaoFoto;        // +76 estado (0..2), +80 resultado (0..8)
};

}  // namespace comum::md
