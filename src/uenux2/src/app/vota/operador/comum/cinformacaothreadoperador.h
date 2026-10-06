// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original (path inferred): uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.h
// Implementation: cinformacaothreadoperador.cpp (attested by srcloc).
#pragma once

#include <optional>
#include <string>

#include "vota/operador/comum/iinformacaothreadoperador.h"

namespace vota {

/// cinformacaothreadoperador.cpp:40 (inlined into wasm func 10586).
std::string TipoToStr(ecourna::app::dados::ETipoIdentificadorEleitor tipo);

/// wasm func 10586 (table slot 3907). Text source of the operator screens: "Título: 1234 5678 9012",
/// "CPF: 123.456.789-01"... built from the identity the mesário typed. name inferred
std::string TextoIdentidadeDigitada();

namespace impl {

/// sizeof == 96 (operator new(96) in wasm func 599).
class CInformacaoThreadOperador final : public IInformacaoThreadOperador {
public:
    CInformacaoThreadOperador();                                                  // inlined into func 599
    ~CInformacaoThreadOperador() override;                                        // func 10552 / 10551

    void LimpaDadosHabilitacao() override;                                        // func 10581
    bool DeveHabilitarAudio() const override;                                     // func 10580
    bool GetAudioAtivado() const override { return m_audioHabilitadoManualmente; }             // icf 2683
    bool EleitorNecessitaAudio() const override;                                  // func 10579
    bool GetAudioHabilitadoManualmente() const override { return m_audioHabilitadoManualmente; } // icf 2683
    void SetAudioHabilitadoManualmente(bool habilitado) override;                 // func 10578
    std::string GetTextoAudio() const override;                                   // func 10577
    bool HabilitadoPorCodigoMesario() const override;                             // func 10576
    bool HabilitadoPorBiometria() const override;                                 // func 10574
    bool HabilitadoSemBiometria() const override;                                 // func 10573
    void SetHabilitacaoCodigoMesario() override;                                  // func 10572
    void SetHabilitacaoBiometrica() override;                                     // func 10571
    void SetHabilitacaoSemBiometria() override;                                   // func 10570
    comum::md::ETipoHabilitacao GetTipoHabilitacao() const override { return m_tipoHabilitacao; } // icf 2587
    std::string GetTextoQtdVotaram() const override;                              // func 10569
    void SorteiaProximaInspecao() override;                                       // func 10568
    api::CDateTime GetDataHoraProximaInspecao() const override;                   // func 10567
    bool VotacaoBloqueadaPorHorario() const override;                             // func 10566
    void LimpaIdentidades() override;                                             // func 10565
    void SetIdentidadeDigitada(const std::string& identidade) override;           // func 10564
    void SetTipoIdentidadeDigitada(ETipoIdentificadorEleitor tipo) override;      // func 10563
    std::string GetIdentidadeDigitada() const override;                           // func 10562
    ETipoIdentificadorEleitor GetTipoIdentidadeDigitada() const override;         // func 10561
    comum::md::CEleitorIdentidade GetIdentidadeEleitor() const override;          // func 10560
    comum::md::CEleitorIdentidade GetIdentidadePrincipalEleitor() const override; // func 10559
    void SetEleitorSemFoto() override;                                            // func 10558
    void SetFotoApresentada() override;                                           // func 10557
    void SetFotoNaoApresentadaPorErro(int resultado) override;                    // func 10556
    ecourna::app::dados::CApresentacaoFotoEleitor GetApresentacaoFoto() const override;   // func 10554

private:
    void AtualizaIdentidadeEleitor();                                             // func 2225, name inferred

    // +0 vptr
    bool                                          m_audioHabilitadoManualmente = false; // +4
    comum::md::ETipoHabilitacao                   m_tipoHabilitacao{};                  // +8  0/1/2
    api::CDateTime                                m_dataHoraProximaInspecao;            // +12 (12 bytes)
    std::optional<std::string>                    m_identidadeDigitada;                 // +24 (flag +36)
    std::optional<ETipoIdentificadorEleitor>      m_tipoIdentidadeDigitada;             // +40 (flag +44)
    std::optional<comum::md::CEleitorIdentidade>  m_identidadeEleitor;                  // +48 (flag +64)
    std::optional<comum::md::CEleitorIdentidade>  m_identidadePrincipal;                // +68 (flag +84)
    int                                           m_estadoApresentacaoFoto = 0;         // +88
    int                                           m_resultadoApresentacaoFoto = 0;      // +92
};

}  // namespace impl
}  // namespace vota
