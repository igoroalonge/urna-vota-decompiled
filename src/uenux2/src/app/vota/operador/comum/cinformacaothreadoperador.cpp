// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original: uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp
// (attested by std::source_location records: lines 40, 474, 484, 494, 504, 554).
//
// Default implementation of the operator thread's blackboard (see iinformacaothreadoperador.h).
// None of these functions ran in the recorded web sessions: the web build never runs the operator
// thread (docs/modules/u10-uenux2-src-app-vota-operador.md, section 2).
#include "vota/operador/comum/cinformacaothreadoperador.h"

#include <format>
#include <random>
#include <string>

#include "api/util/cdatetime.h"
#include "comum/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/crdvvota.h"
#include "comum/dados/md/cvalidadoridentidade.h"
#include "comum/appinfo/cappinfo.h"                       // comum::GetEstado<CEstadoGeralVota>, EhFaseTreinamento...
#include "vota/comum/votadefs.h"                          // vota::CUeVotaError (CBaseError<EUeVotaError>)

namespace vota {

using comum::md::CEleitorIdentidade;
using ecourna::app::dados::ETipoIdentificadorEleitor;

namespace {

// wasm func 5412 (tools: "api_f5412"; also called by the constructor inlined in func 599)   // name inferred
// Picks when the mesário will next be asked to inspect the voting booth ("Inspecione cabina e urna",
// CPedeIdentidade::ProcessTick -> CAguardaInspecao): now + a random 60..90 minutes.
api::CDateTime CalculaHorarioProximaInspecao()
{
    static std::random_device rd;                                   // @1905984 (guard @1905985)
    static std::mt19937 gerador(rd());                              // @1905988 (state), @1908484 (index)
    static std::uniform_int_distribution<int> minutos(60, 90);      // @1908492 = {60, 90}
    api::CDateTime horario = api::CDateTime::Now();                 // func 479
    horario.AdicionaSegundos(minutos(gerador) * 60);                // func 2233
    return horario;
}

}  // namespace

// ---------------------------------------------------------------------------------------------
// cinformacaothreadoperador.cpp:40 - only exists inlined into wasm func 10586.
std::string TipoToStr(ETipoIdentificadorEleitor tipo)
{
    switch (tipo) {
    case ETipoIdentificadorEleitor::TITULO: return "Título";          // Latin-1 "T\xEDtulo" (MT LCD charset)
    case ETipoIdentificadorEleitor::CPF:    return "CPF";
    case ETipoIdentificadorEleitor::LIVRE:  return "Identificador";
    }
    throw CUeVotaError(EUeVotaError{9409}, "Tipo de identidade do eleitor inválido");
}

// wasm func 10586 (tools: "vota::TipoToStr", from the inlined srcloc; table slot 3907)   // name inferred
// Text source used by the operator screens CEleitorVotouNaoVotou, CVerificaDadoEleitor,
// CPedeAnoNascimentoSemBiometria, CNomeEleitor, CEleitorDemorando...
std::string TextoIdentidadeDigitada()
{
    auto& info = impl::IInformacaoThreadOperador::GetInst();
    const CEleitorIdentidade identidade(info.GetIdentidadeDigitada(),             // slot 23
                                        info.GetTipoIdentidadeDigitada());       // slot 24; ctor = func 566
    const std::string tipo = TipoToStr(identidade.GetTipo());
    // ecourna_f1924: "xxxx xxxx xxxx" for a título, "xxx.xxx.xxx-xx" for a CPF, as is otherwise
    return std::format("{}: {}", tipo, comum::md::FormataIdentidadeExibicao(identidade));
}

namespace impl {

// Constructor: inlined into IInformacaoThreadOperador::GetInst (wasm func 599, unit u19).
CInformacaoThreadOperador::CInformacaoThreadOperador()
    : m_dataHoraProximaInspecao(CalculaHorarioProximaInspecao())
{
}

// wasm func 10552 (vtable slot 0) / 10551 (slot 1, deleting)
CInformacaoThreadOperador::~CInformacaoThreadOperador() = default;

// wasm func 10581 (slot 2)                                                   name inferred
// Called by CCancelaHabilitacaoEleitor::StartState when a habilitação is abandoned.
void CInformacaoThreadOperador::LimpaDadosHabilitacao()
{
    m_estadoApresentacaoFoto = 0;
    m_resultadoApresentacaoFoto = 0;
    m_tipoHabilitacao = comum::md::ETipoHabilitacao::SEM_BIOMETRIA;
    m_audioHabilitadoManualmente = false;
}

// wasm func 10580 (slot 3)                                                   name inferred
bool CInformacaoThreadOperador::DeveHabilitarAudio() const
{
    return m_audioHabilitadoManualmente || EleitorNecessitaAudio();
}

// wasm func 10579 (slot 5)                                                   name inferred
// CEleitor::m_necessidadeEspecial (+20) of the voter the roll is positioned on (GetCurrent throws
// when no voter is selected).
bool CInformacaoThreadOperador::EleitorNecessitaAudio() const
{
    return comum::CEleitores::GetInst().GetCurrent().GetEleitor().GetNecessidadeEspecial() == 1;
}

// wasm func 10578 (slot 7)                                                   name inferred
void CInformacaoThreadOperador::SetAudioHabilitadoManualmente(bool habilitado)
{
    m_audioHabilitadoManualmente = habilitado;
}

// wasm func 10577 (slot 8)                                                   name inferred
std::string CInformacaoThreadOperador::GetTextoAudio() const
{
    return m_audioHabilitadoManualmente ? "ÁUDIO ATIVADO"      // Latin-1 "\xC1UDIO ATIVADO" (13 bytes)
                                        : " ";
}

// wasm funcs 10576 / 10574 / 10573 (slots 9 / 10 / 11)                         names inferred
bool CInformacaoThreadOperador::HabilitadoPorCodigoMesario() const
{
    return m_tipoHabilitacao == comum::md::ETipoHabilitacao::CODIGO_MESARIO;   // 2
}
bool CInformacaoThreadOperador::HabilitadoPorBiometria() const
{
    return m_tipoHabilitacao == comum::md::ETipoHabilitacao::BIOMETRIA;        // 1
}
bool CInformacaoThreadOperador::HabilitadoSemBiometria() const
{
    return m_tipoHabilitacao == comum::md::ETipoHabilitacao::SEM_BIOMETRIA;    // 0
}

// wasm funcs 10572 / 10571 / 10570 (slots 12 / 13 / 14)                        names inferred
void CInformacaoThreadOperador::SetHabilitacaoCodigoMesario()
{
    m_tipoHabilitacao = comum::md::ETipoHabilitacao::CODIGO_MESARIO;
}
void CInformacaoThreadOperador::SetHabilitacaoBiometrica()
{
    m_tipoHabilitacao = comum::md::ETipoHabilitacao::BIOMETRIA;
}
void CInformacaoThreadOperador::SetHabilitacaoSemBiometria()
{
    m_tipoHabilitacao = comum::md::ETipoHabilitacao::SEM_BIOMETRIA;
}

// wasm func 10569 (slot 16)                                                  name inferred
// "Votos" counter of the operator's idle screen: 4 digits.
std::string CInformacaoThreadOperador::GetTextoQtdVotaram() const
{
    const unsigned qtd = comum::EhTreinamentoEleitor()                              // func 697
                             ? comum::CRdvVota::GetInst().GetComparecimento()        // func 1269: max over eleições
                             : comum::CEleitores::GetInst().GetQtdVotaram();         // CEleitores +104
    return std::format("{:04}", qtd);
}

// wasm func 10568 (slot 17)                                                  name inferred
void CInformacaoThreadOperador::SorteiaProximaInspecao()
{
    m_dataHoraProximaInspecao = CalculaHorarioProximaInspecao();
}

// wasm func 10567 (slot 18)                                                  name inferred
api::CDateTime CInformacaoThreadOperador::GetDataHoraProximaInspecao() const
{
    return m_dataHoraProximaInspecao;
}

// wasm func 10566 (slot 19)                                                  name inferred
// Polled every minute by CPedeIdentidade::ProcessTick; when true the operator thread logs
// "Votacao foi bloqueada por horario". Never true in the training phase.
bool CInformacaoThreadOperador::VotacaoBloqueadaPorHorario() const
{
    if (comum::EhFaseTreinamento())                                                  // func 1485
        return false;
    const api::CDateTime agora = api::CDateTime::Now();
    // cfg +580: 4th DataHoraJE of ModuloConfiguracaoMunicipios.HorariosUrna (terminoVotacao) ?
    if (agora < comum::CConfiguracaoEleicao::GetInst().GetHorarioTerminoVotacao())
        return false;
    if (comum::CEleitores::GetInst().GetQtdVotaram() == 0)
        return true;
    // cestadogeralvota.h:131 GetDtHrUltimoVoto() throws 8092 "Nenhum eleitor votou" when unset
    api::CDateTime limite = comum::GetEstado<comum::md::estadoaplicacao::CEstadoGeralVota>().GetDtHrUltimoVoto();
    limite.AdicionaSegundos(300);                                                    // func 2233: 5 minutes
    return !(agora < limite);
}

// wasm func 10565 (slot 20)                                                  name inferred
void CInformacaoThreadOperador::LimpaIdentidades()
{
    m_identidadeDigitada.reset();
    m_tipoIdentidadeDigitada.reset();
    m_identidadeEleitor.reset();
    m_identidadePrincipal.reset();
}

// wasm func 10564 (slot 21)                                                  name inferred
void CInformacaoThreadOperador::SetIdentidadeDigitada(const std::string& identidade)
{
    m_identidadeDigitada = identidade;
    AtualizaIdentidadeEleitor();
}

// wasm func 10563 (slot 22)                                                  name inferred
void CInformacaoThreadOperador::SetTipoIdentidadeDigitada(ETipoIdentificadorEleitor tipo)
{
    m_tipoIdentidadeDigitada = tipo;
    AtualizaIdentidadeEleitor();
}

// wasm func 10562 (slot 23) (srcloc line 474)
std::string CInformacaoThreadOperador::GetIdentidadeDigitada() const
{
    if (!m_identidadeDigitada)
        throw CUeVotaError(EUeVotaError{9401}, "Identidade não registrada");
    return *m_identidadeDigitada;
}

// wasm func 10561 (slot 24) (srcloc line 484)
ETipoIdentificadorEleitor CInformacaoThreadOperador::GetTipoIdentidadeDigitada() const
{
    if (!m_tipoIdentidadeDigitada)
        throw CUeVotaError(EUeVotaError{9402}, "Tipo de identidade não registrado");
    return *m_tipoIdentidadeDigitada;
}

// wasm func 10560 (slot 25) (srcloc line 494)
CEleitorIdentidade CInformacaoThreadOperador::GetIdentidadeEleitor() const
{
    if (!m_identidadeEleitor)
        throw CUeVotaError(EUeVotaError{9403}, "Identidade não registrada");
    return *m_identidadeEleitor;
}

// wasm func 10559 (slot 26) (srcloc line 504)
CEleitorIdentidade CInformacaoThreadOperador::GetIdentidadePrincipalEleitor() const
{
    if (!m_identidadePrincipal)
        throw CUeVotaError(EUeVotaError{9404}, "Identidade não registrada");
    return *m_identidadePrincipal;
}

// wasm funcs 10558 / 10557 / 10556 (slots 27 / 28 / 29)                        names inferred
// Values of ModuloResultadoUrnaCadastro.ApresentacaoFotoEleitor (estado 0 sem foto, 1 apresentada,
// 2 não apresentada por erro + ResultadoApresentacaoFotoEleitor).
void CInformacaoThreadOperador::SetEleitorSemFoto()
{
    m_estadoApresentacaoFoto = 0;
    m_resultadoApresentacaoFoto = 0;
}
void CInformacaoThreadOperador::SetFotoApresentada()
{
    m_estadoApresentacaoFoto = 1;
    m_resultadoApresentacaoFoto = 0;
}
void CInformacaoThreadOperador::SetFotoNaoApresentadaPorErro(int resultado)
{
    m_resultadoApresentacaoFoto = resultado;
    m_estadoApresentacaoFoto = 2;
}

// wasm func 10554 (slot 30)                                                  name inferred
ecourna::app::dados::CApresentacaoFotoEleitor CInformacaoThreadOperador::GetApresentacaoFoto() const
{
    return {m_estadoApresentacaoFoto, m_resultadoApresentacaoFoto};                  // shared_f1081 (2-int ctor)
}

// wasm func 2225 (tools: "vota_f2225")                                       name inferred
// Recomputes the voter identity once both the typed number and its type are known, and derives the
// voter's *principal* identity (the key of the roll): the same identity when the typed type is the
// configured principal type, otherwise the principal identity of the voter found through the
// secondary index CEleitores::m_identidadePrincipal (map at +112). Side effect: positions the roll
// (CDataMap current iterator, +16) on that voter.
void CInformacaoThreadOperador::AtualizaIdentidadeEleitor()
{
    if (!m_identidadeDigitada || !m_tipoIdentidadeDigitada)
        return;
    // vota_f2803: finds the rule of that type (or a rule of type 3) and checks the digits
    if (!comum::md::CValidadorIdentidade::GetInst().EhValida(*m_tipoIdentidadeDigitada, *m_identidadeDigitada))
        return;

    m_identidadeEleitor = CEleitorIdentidade(*m_identidadeDigitada, *m_tipoIdentidadeDigitada);   // func 566

    if (*m_tipoIdentidadeDigitada == comum::CConfiguracaoEleicao::GetInst().GetTipoIdentificadorPrincipal()) {   // cfg +668
        m_identidadePrincipal = m_identidadeEleitor;
        return;
    }

    const comum::CEleitorDetalhe* eleitor =
        comum::CEleitores::GetInst().PosicionaPorIdentidadeSecundaria(*m_identidadeEleitor);   // inlined, name inferred
    if (eleitor != nullptr) {
        const auto& decorator = eleitor->GetEleitor();
        m_identidadePrincipal = decorator.GetIdentidadePorTipo(decorator.GetTipoIdentificadorPrincipal());   // +104
    } else {
        m_identidadePrincipal.reset();
    }
}

}  // namespace impl
}  // namespace vota
