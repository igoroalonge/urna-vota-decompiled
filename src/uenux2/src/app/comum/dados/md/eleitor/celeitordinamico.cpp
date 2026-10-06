// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/eleitor/celeitordinamico.cpp
#include "celeitordinamico.h"

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 5663 (constructor; the unit tool attributed it to celeitordinamicodao.cpp, its only caller)
CEleitorDinamico::CEleitorDinamico(CEleitorIdentidade titulo, CEleitorIdentidade identidadeHabilitacao,
                                   int estadoComparecimento, int tipoHabilitacao, int dedoHabilitacao,
                                   std::uint16_t scoreHabilitacao, std::uint8_t numeroTentativa,
                                   int tipoAtivacaoAudio, int erroDecifrarBiometria,
                                   CEleitorIdentidade tituloMesarioHabilitacao,
                                   TApresentacaoFoto apresentacaoFoto)
    : m_titulo(titulo)
    , m_identidadeHabilitacao(identidadeHabilitacao)
    , m_estadoComparecimento(estadoComparecimento)
    , m_tipoHabilitacao(tipoHabilitacao)
    , m_dedoHabilitacao(dedoHabilitacao)
    , m_scoreHabilitacao(scoreHabilitacao)
    , m_numeroTentativa(numeroTentativa)
    , m_tipoAtivacaoAudio(tipoAtivacaoAudio)
    , m_erroDecifrarBiometria(erroDecifrarBiometria)
    , m_tituloMesario(tituloMesarioHabilitacao)
    , m_apresentacaoFoto(apresentacaoFoto)
{
}

// wasm func 5664 (constructor without poll-worker override)
CEleitorDinamico::CEleitorDinamico(CEleitorIdentidade titulo, CEleitorIdentidade identidadeHabilitacao,
                                   int estadoComparecimento, int tipoHabilitacao, int dedoHabilitacao,
                                   std::uint16_t scoreHabilitacao, std::uint8_t numeroTentativa,
                                   int tipoAtivacaoAudio, int erroDecifrarBiometria,
                                   TApresentacaoFoto apresentacaoFoto)
    : m_titulo(titulo)
    , m_identidadeHabilitacao(identidadeHabilitacao)
    , m_estadoComparecimento(estadoComparecimento)
    , m_tipoHabilitacao(tipoHabilitacao)
    , m_dedoHabilitacao(dedoHabilitacao)
    , m_scoreHabilitacao(scoreHabilitacao)
    , m_numeroTentativa(numeroTentativa)
    , m_tipoAtivacaoAudio(tipoAtivacaoAudio)
    , m_erroDecifrarBiometria(erroDecifrarBiometria)
    , m_tituloMesario(std::nullopt)
    , m_apresentacaoFoto(apresentacaoFoto)
{
}

// wasm func 1548 (srcloc line 93)
const CEleitorIdentidade& CEleitorDinamico::GetTituloMesarioHabilitacao() const
{
    if (m_tituloMesario.has_value())
        return *m_tituloMesario;
    throw CUeComumDadosError(8067, "Eleitor não habilitado manualmente");
}

}  // namespace comum::md
