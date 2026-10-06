// Reconstructed from vota_web_wasm.wasm (unit u27). Original: uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp
// (attested: srcloc celeitorencontrado.cpp:102 inside StartState; srcloc clogvota.cpp:432
// = CLogVota::LogaEleitorImpedido, inlined).
//
// Functions:
//   10635  CEleitorEncontrado::StartState (vtable slot 2)
//   3769   impediment of the current round (tools: vota_f3769)            name inferred
//   4578   "justification not accepted" (tools: vota_f4578)                name inferred
// Attributed here by the tools but moved to leidentidade/ieleitorimpedidovotar.cpp (path inferred):
//   1905 IEleitorImpedidoVotar ctor, 1257 dtor, 5419 form helper, 5424 CEleitorNaoEncontrado::GetInst.
//
// WEB BUILD: dead code; reached in the harness run of u27 (CProcuraEleitor -> CEleitorEncontrado ->
// CNomeEleitor for the only voter of municipal-t1).
#include "vota/operador/leidentidade/celeitorencontrado.h"

#include <format>
#include <set>
#include <string>

#include "api/gui/cformbuildermt.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "comum/informacao/cinformacaoeleicao.h"
#include "vota/log/clogvota.h"
#include "vota/operador/confirmaidentidade/cnomeeleitor.h"
#include "vota/operador/justificativa/cconfirmajustificativa.h"
#include "vota/operador/justificativa/ciniciajustificativa.h"
#include "vota/operador/leidentidade/celeitorjavotou.h"
#include "vota/operador/leidentidade/ieleitorimpedidovotar.h"

namespace vota {

using api::SPoint;
using comum::md::ETipoImpedimento;

namespace {

constexpr auto ESQUERDA = api::ETextAlignment(0);
constexpr auto DIREITA  = api::ETextAlignment(1);

// md enum = ASN.1 ModuloImpedidos::TipoImpedimento - 1 (0-based). Names from the ASN.1 enumerators.
enum : int {
    SEM_IMPEDIMENTO = 0, VOTA_NA_SECAO_ORIGINAL = 1, SOLICITOU_VOTO_EM_TRANSITO = 2, PRESO_PROVISORIO = 3,
    SUSPENSO = 4, CANCELADO = 5, NAO_LIBERADO = 6, SEM_IDADE_MINIMA = 7, MILITAR_EM_SERVICO = 8,
    TTE_ACESSIBILIDADE = 9, TTE_ELEITOR_CONVOCADO = 10, TTE_JUSTICA_ELEITORAL = 11, TTE_OFICIO = 12,
    TTE_INDIGENAS_QUILOMBOLAS = 13, TTE_SITUACAO_RUA = 14,
};

// wasm func 3769 (tools: vota_f3769). Impediment of the round being voted: CEleitorDetalhe +200 in the
// 2nd round (CEstadoGeral turno == '2'), +196 otherwise. Probably an inline member of CEleitorDetalhe
// emitted out of line (comum_f2266 = "== 0" of the same expression).                  name inferred
int ImpedimentoTurnoAtual(const comum::CEleitorDetalhe& eleitor)
{
    const bool segundoTurno = comum::GetEstadoGeral(comum::CAppInfo::GetInst()).GetTurno() == '2';   // 457 +8
    return static_cast<int>(segundoTurno ? eleitor.GetImpedimentoP2() : eleitor.GetImpedimentoP1());
}

// wasm func 4578 (tools: vota_f4578)                                                 name inferred
// ParametrosUrna.aceitarJustificativa (cfg +482) through CInformacaoEleicao (func 5918).
bool JustificativaNaoAceita()
{
    return !comum::CInformacaoEleicao(comum::CConfiguracaoEleicao::GetInst()).ImprimeBoletimJustificativa();
}

// Inlined CLogVota::LogaEleitorImpedido(comum::md::ETipoImpedimento) (clogvota.cpp:432).
void LogaEleitorImpedido(CLogVota& log, const int tipo)
{
    std::string motivo;
    switch (tipo) {
    case SEM_IMPEDIMENTO:           return;                                    // nothing logged
    case VOTA_NA_SECAO_ORIGINAL:    motivo = "vota na seção original"; break;
    case SOLICITOU_VOTO_EM_TRANSITO: motivo = "solicitou voto em trânsito"; break;
    case PRESO_PROVISORIO:          motivo = "preso provisório"; break;
    case SUSPENSO:                  motivo = "título suspenso"; break;
    case CANCELADO:                 motivo = "título cancelado"; break;
    case NAO_LIBERADO:              motivo = "eleitor não está liberado para votar"; break;
    case SEM_IDADE_MINIMA:          motivo = "eleitor não tem idade mínima para votar"; break;
    case MILITAR_EM_SERVICO:        motivo = "eleitor é militar em serviço"; break;
    case TTE_ACESSIBILIDADE:        motivo = "eleitor pediu acessibilidade"; break;
    case TTE_ELEITOR_CONVOCADO:     motivo = "eleitor convocado pela Justiça Eleitoral"; break;
    case TTE_JUSTICA_ELEITORAL:     motivo = "eleitor é servidor da Justiça Eleitoral"; break;
    case TTE_OFICIO:                motivo = "eleitor transferido por ofício"; break;
    case TTE_INDIGENAS_QUILOMBOLAS: motivo = "eleitor é um membro da comunidade indígena ou quilombola"; break;
    case TTE_SITUACAO_RUA:          motivo = "eleitor está em situação de rua"; break;
    case 15:                                                                   // clogvota.cpp:432
        throw CUeVotaError(EUeVotaError{9388}, "Tipo inválido");
    default:                        break;                                     // >= 16: logged with an empty reason
    }
    log.Loga(std::format("Eleitor impedido - {}", motivo));                    // level 1
}

// Voter-in-transit justification is offered when the configuration holds a non-zero entry in the
// std::set<int> at CConfiguracaoEleicao +604 (copied with the range constructor, func 6733, destroyed by
// func 1594). The meaning of the set is not identified here.                          ?
bool PermiteJustificativaTransito()
{
    const auto& cfg = comum::CConfiguracaoEleicao::GetInst();
    const std::set<int> valores(cfg.GetConjunto604().begin(), cfg.GetConjunto604().end());   // name unknown
    return std::ranges::any_of(valores, [](int v) { return v != 0; });
}

}  // namespace

// Lazy singleton @1905616 (inlined into CProcuraEleitor::StartState, func 10631).
CEleitorEncontrado& CEleitorEncontrado::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CEleitorEncontrado> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CEleitorEncontrado());
    return *s_inst;
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10635 (vtable slot 2, srcloc :102)
void CEleitorEncontrado::StartState()
{
    const comum::CEleitorDetalhe* eleitor = comum::CEleitores::GetInst().GetCurrentPtr();   // CDataMap::GetCurrent (656)
    if (eleitor == nullptr)
        throw CUeVotaError(EUeVotaError{9394}, "Eleitor não posicionado");                   // line 102

    if (!eleitor->PodeVotar()) {                                   // comum_f2266: impediment of this round != 0
        m_proximoEstado = EstadoEleitorImpedido(*eleitor);
        return;
    }

    switch (eleitor->GetDinamico().GetEstado()) {                  // CEleitorDinamico +32 (func 1271)
    case comum::md::EEstadoComparecimento::SEM_CARGO_PARA_VOTAR:   // 1: voter in transit, no cargo here
        m_proximoEstado = &CEleitorNaoPossuiCargosParaVotar::GetInst();     // "não está apto a votar nesta eleição."
        return;
    case comum::md::EEstadoComparecimento::VOTOU:                  // 3
        m_proximoEstado = &CEleitorJaVotou::GetInst();
        //   ^ lazy singleton @1905560 (mutex @1905536), 20 bytes, constructor inlined here: CAppState(2);
        //     LED off; (1,1) name CDataTextFmt(CEleitorDadoNomeParaUrna::Text, "{:2}") (slot 3906);
        //     (1,2) typed identity (slot 3907); (1,3) "JÁ VOTOU"; (40,4) right "CONFIRMA: prosseguir";
        //     input control; interactive form (+12). ProcessInput = func 10641 (u19).
        return;
    default:
        m_proximoEstado = &CNomeEleitor::GetInst();                 // func 5401: name + photo, habilitação
        return;
    }
}

// Inlined into 10635.                                                                  name inferred
comum::CAppState* CEleitorEncontrado::EstadoEleitorImpedido(const comum::CEleitorDetalhe& eleitor)
{
    const int impedimento = ImpedimentoTurnoAtual(eleitor);                    // func 3769
    LogaEleitorImpedido(CLogVota::GetInst(), impedimento);

    if (JustificativaNaoAceita()) {                                            // func 4578
        if (ImpedimentoTurnoAtual(eleitor) == SOLICITOU_VOTO_EM_TRANSITO)
            return &CEleitorOptouPorVotarEmTransito::GetInst();                // "Optou por votar em trânsito"
        return &CEleitorNaoEncontrado::GetInst();                              // func 5424
    }

    const int tipo = ImpedimentoTurnoAtual(eleitor);
    if (tipo >= SUSPENSO && tipo <= NAO_LIBERADO)                              // (tipo - 7) >= -3 unsigned
        return &CEleitorImpedidoJustificar::GetInst();                         // cannot vote NOR justify
    if (tipo == SEM_IDADE_MINIMA)
        return &CEleitorNaoTemIdadeMinima::GetInst();
    if (tipo == SOLICITOU_VOTO_EM_TRANSITO) {
        if (!PermiteJustificativaTransito())
            return &CEleitorImpedidoJustificarVotoTransito::GetInst();          // "Eleitor solicitou voto em trânsito"
        return &CIniciaJustificativaTransito::GetInst();
        //   ^ @1905840 (mutex @1905816): IIniciaJustificativa(&CConfirmaJustificativaTransito::GetInst());
        //     CConfirmaJustificativaTransito @1905812 = IConfirmaJustificativa("optou por votar EM TRÂNSITO").
    }
    // vota na seção original, preso provisório, militar, TTE ...: the voter may justify his absence here.
    return &CIniciaJustificativaTemporario::GetInst();
    //   ^ @1905784 (mutex @1905760): IIniciaJustificativa(&CConfirmaJustificativaTemporario::GetInst());
    //     CConfirmaJustificativaTemporario @1905756 = IConfirmaJustificativa("está impedido de votar nesta seção").
}

}  // namespace vota
