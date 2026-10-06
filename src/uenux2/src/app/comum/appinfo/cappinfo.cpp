// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/app/comum/appinfo/cappinfo.cpp (srclocs cappinfo.cpp:42, :57, :472).
// The free functions at the end (SalvaEstado & co.) have no srcloc of their own; they are placed here
// because they only combine CAppInfo getters (path inferred).
#include "comum/appinfo/cappinfo.h"

#include <format>
#include <optional>
#include <string>
#include <vector>

#include "api/gui/capplicationcontextstack.h"
#include "api/util/csynchronizer.h"
#include "api/util/csystem.h"
#include "comum/appinfo/servicos/cservicoestadogeralvota.h"
#include "comum/cinformacaoeleicao.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/gravadores/cassinador.h"
#include "comum/nomearquivo/carquivossavd.h"
#include "vota/comum/csincronizavota.h"

namespace comum {

using md::estadoaplicacao::CEstadoGeral;
using md::estadoaplicacao::CEstadoGeralGap;
using md::estadoaplicacao::CEstadoGeralVota;

namespace {

// srcloc cappinfo.cpp:42 (non-const, error 7600) - inlined / merged, see below.
template <typename ESTADO>
ESTADO& GetEstado(const std::string& nome, std::optional<ESTADO>& estado)
{
    if (!estado)
        throw CUeComumAppInfoError(EUeComumAppInfoError{7600},
                                   std::format("O estado não foi carregado: {}", nome));
    return *estado;
}

// srcloc cappinfo.cpp:57 (const, error 7601)
template <typename ESTADO>
const ESTADO& GetEstado(const std::string& nome, const std::optional<ESTADO>& estado)
{
    if (!estado)
        throw CUeComumAppInfoError(EUeComumAppInfoError{7601},
                                   std::format("O estado não foi carregado: {}", nome));
    return *estado;
}

} // namespace

// wasm func 1553 - observed executing                                       srcloc cappinfo.cpp:472
// The message formats the ARGUMENT `turno` (stored to the stack before the '3' -> turno-atual resolution,
// handle arg @c+44), not the resolved value: IndiceTurno(f, Atual) on an urna "sem turno" reports
// "Turno 3 invalido", not "Turno 0".
std::size_t CAppInfo::IndiceTurno(const std::string& funcao, EUrnaTurno turno)
{
    const EUrnaTurno efetivo = (turno == EUrnaTurno::Atual)
        ? GetInst().GetGeral().GetTurno()                         // CEstadoGeral +32 (non-const GetGeral, 291)
        : turno;
    switch (efetivo) {
    case EUrnaTurno::Primeiro: return 0;
    case EUrnaTurno::Segundo:  return 1;
    default:
        throw CUeComumAppInfoError(EUeComumAppInfoError{7602},
                                   std::format("Turno {} invalido: {}", turno, funcao));   // EUrnaTurno formatter
    }
}

// wasm func 6040 (tools: comum_f6040) - merged body of both GetGeral() overloads
// (srcloc record and error code passed as parameters by the thunks 291 / 457). Observed executing.
// wasm func 291 - observed executing (35 table slots, 51 callers)
CEstadoGeral& CAppInfo::GetGeral()
{
    return GetEstado("GetGeral", m_geral);                        // 6040(this, srcloc :42, 7600)
}

// wasm func 457 - observed executing
const CEstadoGeral& CAppInfo::GetGeral() const
{
    return GetEstado("GetGeral", m_geral);                        // 6040(this, srcloc :57, 7601)
}

// wasm func 261 (tools: comum::GetEstado@261) - observed executing                   name inferred
CEstadoGeralVota& CAppInfo::GetVota(EUrnaTurno turno)
{
    return GetEstado("GetVota", m_vota[IndiceTurno("GetVota", turno)]);
}

// wasm func 903 (tools: comum::GetEstado@903)                                         name inferred
// Callers: CAjusteInicial, CEncerramentoHorarioInvalido, CVerificaHorarioZeresima, CEmitirMaisBU, the
// operator's "parâmetros da urna / versões / lista de eleitores / estado da urna" items ...
const CEstadoGeralVota& CAppInfo::GetVota() const
{
    return GetEstado("GetVota", m_vota[IndiceTurno("GetVota", EUrnaTurno::Atual)]);
}

// wasm func 2841 (tools: comum_f2841). Callers: CGeraBU, CGeraRelatorios, CGeradorBUQRCodeVota. name inferred
bool CAppInfo::TemVota(EUrnaTurno turno) const
{
    return m_vota[IndiceTurno("TemVota", turno)].has_value();
}

// wasm func 3788 (tools: comum::CAppInfo::GetHistoricoCargas). Callers: vota::CInformacaoEleitor::Inicializar
// (7787, votaInit), CGeraBU, CGravaResultado. Returns the carga codes (CDadoCorrespondencia +28, elements of
// 96 bytes) of the correspondences recorded in gap.bin: the "histórico de cargas" printed in the BU / QR code (HIQT/HICA).
std::vector<std::string> CAppInfo::GetHistoricoCargas() const                          // name inferred
{
    const CEstadoGeralGap& gap = GetEstado("GetGap", m_gap[IndiceTurno("GetGap", EUrnaTurno::Atual)]);
    std::vector<std::string> codigos;
    codigos.reserve(gap.GetCorrespondencias().size());
    for (const auto& correspondencia : gap.GetCorrespondencias())
        codigos.push_back(correspondencia.GetCodigoCarga());
    return codigos;
}

// wasm func 6039 (tools: comum_f6039) - observed executing. Merged body of SalvaVotaInterno/Externo:
// the 16-character function name is passed as two 8-byte halves. Writes vota.bin of the current turno
// through CServicoEstadoGeralVota(midia, turno) - nothing happens when the state is not loaded.
void CAppInfo::SalvaVota(const std::string& funcao, int midia)                         // name inferred
{
    auto& estado = m_vota[IndiceTurno(funcao, EUrnaTurno::Atual)];
    if (estado)
        CServicoEstadoGeralVota(midia, EUrnaTurno::Atual).Salva(*estado);   // 3787 ctor, 5329 Salva
}

// wasm func 3790 (tools: unknown_f3790)   MI = internal flash                        name inferred
void CAppInfo::SalvaVotaInterno() { SalvaVota("SalvaVotaInterno", 0); }

// wasm func 3789 (tools: unknown_f3789)   MV = external flash (voting media)         name inferred
void CAppInfo::SalvaVotaExterno() { SalvaVota("SalvaVotaExterno", 1); }

// =====================================================================================================
// Free helpers (path inferred)
// =====================================================================================================

// wasm func 1823 (tools: vota_f1823): training of poll workers ("treinamento do mesário"), i.e. phase
// '3' (treinamento) without EstadoGeralVota.treinamentoEleitor. Callers: CIniciodeCiclo::AjustaDataHora,
// CPedeDigital::ProcessTick, CDigitalNaoReconhecida*, CNomeEleitor, CControlaReconhecimento.
bool EhTreinamentoSemTreinamentoEleitor()                                              // name as in u10
{
    auto& app = CAppInfo::GetInst();
    return app.GetGeral().GetFase() == EFase::Treinamento           // +48 == '3'
        && !app.GetVota(EUrnaTurno::Atual).EhTreinamentoEleitor();  // +72
}

// wasm func 2520 (tools: comum_f2520). Callers: CFimAquisicaoVotos::StartState, CQuerReimprimirZeresima,
// CDefineRotaPosReinicio::NeedChangeState, api_f2753.       name inferred (u09: DeveRegistrarMesarios)
bool EhModoDemonstracaoSemTreinamentoEleitor()
{
    if (!CInformacaoEleicao(CConfiguracaoEleicao::GetInst() /* +88 */).EhModoDemonstracao())   // func 1950
        return false;
    auto& app = CAppInfo::GetInst();
    if (app.GetGeral().GetFase() != EFase::Treinamento)
        return true;
    return !app.GetVota(EUrnaTurno::Atual).EhTreinamentoEleitor();
}

// wasm func 4687 (tools: comum_f4687) - observed executing. Copies the vota.vsu signature package of
// the internal flash to the external one (ESavdPacote 122 -> 124 in turno 1, 123 -> 125 in turno 2).
void CopiaAssinaturaEstadoVotaParaMV()                                                 // name inferred
{
    auto& arquivos = CArquivosSavd::GetInst();                                          // func 1164
    if (CAppInfo::GetInst().GetGeral().GetTurno() == EUrnaTurno::Primeiro)
        api::CSystem::CopyFile(arquivos[ESavdPacote{122}], arquivos[ESavdPacote{124}], false);
    else
        api::CSystem::CopyFile(arquivos[ESavdPacote{123}], arquivos[ESavdPacote{125}], false);
}

// wasm func 491 (tools: comum_f491) - observed executing; 32 callers (every change of estadoVota:
// init, zerésima, mesário registration, encerramento, BU printing ...). Other units call it
// comum::SalvaEstado() / SalvaEstadoVota().                                            name inferred
// Persists vota.bin on BOTH flashes, each step under an application context that the fatal-error screen
// shows if something throws:
//   MI: write vota.bin, sign it into the vota.vsu package (CAssinador(122|123).Assina(31 = vota.bin)), sync
//   MV: write vota.bin, copy the MI signature package, sync
void SalvaEstado()
{
    std::optional<api::CApplicationContext> contextoAtual;
    auto& pilha = api::CApplicationContextStack::GetInst();                  // vector @1839212
    if (!pilha.Empty())
        contextoAtual = pilha.Top();                                          // func 1841 (copy)
    const std::string detalhe = contextoAtual ? contextoAtual->m_detalhe : "Erro de sincronização";

    auto& app = CAppInfo::GetInst();
    {
        api::CApplicationContextGuard contexto(api::Actions::ReinicieOuSubstituaUrna /*2*/, detalhe,
            "Gravando o estado da urna",
            "Ocorreu um erro durante a sincronização do estado geral do VOTA.");          // func 676
        vota::CSincronizaVota::VerificaUrnaDesligando();   // inlined: flag @1832936 -> CUeDesligandoError 4201
        app.SalvaVotaInterno();                                                          // func 3790
        CAssinador assinador(app.GetGeral().GetTurno() == EUrnaTurno::Primeiro ? ESavdPacote{122}
                                                                               : ESavdPacote{123});  // 1501
        assinador.Assina(ESavdArquivoUE{31});                                            // vota.bin, func 1277
        api::CSynchronizer::GetInst().Sync();
    }                                                                                    // func 675
    {
        api::CApplicationContextGuard contexto(api::Actions::ReinicieOuSubstituaMidiaVotacao /*4*/, detalhe,
            "Gravando o estado da urna",
            "Ocorreu um erro durante a sincronização do estado geral do VOTA.");
        app.SalvaVotaExterno();                                                          // func 3789
        CopiaAssinaturaEstadoVotaParaMV();                                               // func 4687
        api::CSynchronizer::GetInst().Sync();
    }
}

} // namespace comum
