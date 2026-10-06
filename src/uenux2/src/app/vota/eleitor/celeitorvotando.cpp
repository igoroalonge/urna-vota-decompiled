// Reconstructed from vota_web_wasm.wasm (unit u06). Original: uenux2/src/app/vota/eleitor/celeitorvotando.cpp
// (build path /home/rubio/tse/uenux2/src/app/vota/eleitor/celeitorvotando.cpp)
//
// srcloc evidence:
//   :125 / :131  void vota::CEleitorVotando::IniciaCiclo()               (IScreen / IInputKbd lookups)
//   :229         void vota::CEleitorVotando::ChamaEstadoProximoCargo()
//   :253 / :254  void vota::CEleitorVotando::DescartaVotos()             (IBeep / ISound lookups)
//   :294         virtual bool vota::CEleitorVotando::NeedChangeState()   (IBeep lookup)
//   :410 / :427  void vota::CEleitorVotando::suspenderEleitor(const EOrigemSuspensao)
//
// Error type: CUeVotaError = ecourna::api::exception::CBaseError<vota::EUeVotaError, ...>
//             (typeinfo @1532388, factory func 253).
//
// Several functions that the analyzer attributed to this file are not CEleitorVotando code. They
// are reconstructed at the end (section "Functions emitted in this translation unit") with their
// probable real owner.

#include "vota/eleitor/celeitorvotando.h"

#include <algorithm>
#include <format>

#include "api/hwil/ibeep.h"
#include "api/hwil/iinputkbd.h"
#include "api/hwil/iscreen.h"
#include "api/hwil/isound.h"
#include "comum/cappinfo.h"
#include "comum/dados/ccargos.h"
#include "comum/dados/ccandidaturas.h"          // ? singleton wasm func 521
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "vota/comum/cthreadoperador.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/eleitor/comum/cinformacaoeleitor.h"   // CInformacaoEleitor (func 509, unit u02)
#include "vota/eleitor/cinstrucaovotacaoacessibilidade.h"
#include "vota/eleitor/cconfirmavotosemcandidato.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/votamajoritario/cpedemajoritario.h"
#include "vota/eleitor/votaproporcional/cpedeproporcional.h"
#include "vota/log/clogvota.h"
#include "vota/operador/caguardamensagem.h"

namespace vota {

std::string g_votoDigitado;                                         // @1833288, dtor func 13593
std::vector<std::pair<TCargoID, comum::md::CVoto>> g_votosEleitor;  // @1833300, dtor func 13588
uebyte g_numeroEscolha = 1;                                         // @1536340 (initialised data)

namespace {

/// Tells the operator terminal which cargo the voter is on: "VOTANDO PARA: <cargo>[ - <n>ª vaga]",
/// at most 40 characters (the terminal line). wasm func 4464. name inferred
void InformaCargoAoOperador(const comum::md::CCargo& cargo)
{
    std::string texto;
    if (cargo.EhCargoEletivo()) {                                   // CCargo +84
        const auto monta = [&](const std::string& nome) {
            return cargo.GetQtdEscolhas() == 1                      // CCargo +13
                       ? nome
                       : nome + " - " + cargo.GetOrdinalEscolha(g_numeroEscolha);   // "{}ª vaga"
        };
        texto = "VOTANDO PARA: " + monta(cargo.GetNome());          // func 1547 (CCargo +24)
        if (texto.size() > 40)
            texto = "VOTANDO PARA: " + monta(cargo.GetNomeAbreviado());   // func 2797 (CCargo +60)
    } else {
        // consulta (referendum question): short name only, cut to 40 characters
        texto = "VOTANDO PARA: " + cargo.GetNomeAbreviado();
        if (texto.size() > 40)
            texto.resize(40);
    }

    auto& operador = CThreadOperador::GetInst();
    operador.m_textoCargo = texto.substr(0, 40);                    // CThreadOperador +108
    operador.EnviaMensagem({EMensagemOperador::AtualizaCargoAtual}, 1);
}

}  // namespace

// wasm func 2483 (slot 0) / 7385 (slot 1, deleting)
CEleitorVotando::~CEleitorVotando() = default;   // only m_contexto (func 1468) is destroyed

// ---------------------------------------------------------------------------------------------
// wasm func 7377 — vtable slot 2. The analyzer named it IniciaCiclo because IniciaCiclo() is
// inlined here (srcloc 125/131); the virtual itself is StartState(). Observed executing.
void CEleitorVotando::StartState()
{
    // error context shown if an exception escapes while this voter is voting
    api::CApplicationContextStack::Push(api::CApplicationContext(m_contexto));   // funcs 1841, 3684
    CThreadOperador::GetInst().EnviaMensagem({EMensagemOperador::EleitorIniciouVotacao}, 1);
    IniciaCiclo();
}

// inlined into func 7377
void CEleitorVotando::IniciaCiclo()
{
    auto& tela = api::IScreen::GetInst();                           // srcloc :125
    tela.Clear(1);                                                  // IScreen slot 4
    tela.Refresh();                                                 // slot 27
    tela.vf14();                                                    // slot 14 ? (no-op in CWasmScreen)

    auto& teclado = api::IInputKbd::GetInst();                      // srcloc :131
    teclado.vf7();                                                  // slot 7 ? (no-op in CWasmInputKbd)

    CThreadEleitor::GetInst().StartTick(m_tickEleitorDemorando);
    m_eleitorDemorando = false;
    m_vemDaInstrucaoAcessibilidade = false;
    m_proximoEstado = this;

    g_votoDigitado.clear();
    g_votosEleitor.clear();
    g_numeroEscolha = 1;

    // Voto em trânsito: the voter's abrangência restricts the cargos (func 3721, name inferred):
    //   0 = voter of this município (all cargos), 1 = other município same UF (estadual + federal),
    //   2 = other UF (federal only).
    const auto& eleitores = comum::CEleitores::GetInst();           // throws "CEleitores - instancia
                                                                    //  nao criada" / "Não posicionado
                                                                    //  no eleitor corretamente"
    const auto abrangencia = eleitores.GetEleitorAtual().GetAbrangencia(eleitores.GetUF(),
                                                                       eleitores.GetMunicipio());
    auto& cargos = comum::CCargos::GetInst();
    cargos.FiltraPorAbrangencia(abrangencia);   // inlined (ccargos.cpp:249, "Tipo de abrangência
                                                // inválido: {}"), then sorted by eleição/ordem
                                                // (comparator func 11559) and rewound (position 0)
    teclado.Clear();                                                // IInputKbd slot 4

    if (cargos.IsEnd()) {
        // no cargo for this voter: drop the sub-state; NeedChangeState() returns true next time.
        // NOTE: m_proximoEstado is still `this`, so the thread restarts CEleitorVotando (see doc).
        if (m_estadoCargo)
            m_estadoCargo->FinishState();
        m_estadoCargo = nullptr;
        return;
    }

    if (CInformacaoEleitor::GetInst().m_modoAudio != 2) {    // func 509 (+4): 2 = áudio desabilitado
        // accessibility: first play the keyboard instructions
        auto& operador = CThreadOperador::GetInst();
        operador.m_textoCargo = "Tela de instrução de acessibilidade";
        operador.EnviaMensagem({EMensagemOperador::AtualizaCargoAtual}, 1);

        auto* instrucao = &CInstrucaoVotacaoAcessibilidade::GetInst();   // func 4420
        if (m_estadoCargo)
            m_estadoCargo->FinishState();
        m_estadoCargo = instrucao;
        g_votoDigitado = "";
        m_vemDaInstrucaoAcessibilidade = true;
        m_estadoCargo->StartState();
        return;
    }

    InformaCargoAoOperador(cargos.GetCurrent());
    ChamaEstadoProximoCargo();
}

// ---------------------------------------------------------------------------------------------
// wasm func 4459 — srcloc :229. Observed executing.
void CEleitorVotando::ChamaEstadoProximoCargo()
{
    const auto& cargo = comum::CCargos::GetInst().GetCurrent();
    const std::vector<int> candidatos =
        comum::CCandidaturas::GetInst().GetNumerosCandidatos(cargo.GetId());   // ecourna_f2840 ?

    comum::CAppState* estado = nullptr;
    if (candidatos.empty() && !cargo.EhConsulta()) {                // CCargo +136
        estado = &CConfirmaVotoSemCandidato::GetInst();             // inline singleton (32 bytes)
    } else if ((cargo.EhCargoEletivo() && cargo.GetTipo() == comum::md::ETipoCargo::Majoritario) ||
               cargo.EhConsulta()) {
        estado = &CPedeMajoritario::GetInst();                      // func 5921
    } else if (cargo.EhCargoEletivo() && cargo.GetTipo() == comum::md::ETipoCargo::Proporcional) {
        estado = &CPedeProporcional::GetInst();                     // func 3849
    } else {
        CLogVota::GetInst().LogaErro("Erro na identificação do tipo de cargo");   // severity 3
        throw CUeVotaError(9310, "Tipo de cargo nao identificado", std::source_location::current());
    }

    if (m_estadoCargo)
        m_estadoCargo->FinishState();
    m_estadoCargo = estado;
    g_votoDigitado = std::string{};
    if (m_estadoCargo)
        m_estadoCargo->StartState();
}

// ---------------------------------------------------------------------------------------------
// wasm func 4449 — srcloc :253/:254. The voter leaves without any recorded vote.
void CEleitorVotando::DescartaVotos()
{
    CThreadOperador::GetInst().EnviaMensagem({EMensagemOperador::EleitorNaoVotou}, 1);
    CTelasVota::GetInst().m_telaFimNaoVotou->Exibe();              // CTelasVota +164: "FIM / NÃO VOTOU"
    m_proximoEstado = &CAguardaMensagem::GetInst();                 // func 1337
    if (m_estadoCargo)
        m_estadoCargo->FinishState();
    m_estadoCargo = nullptr;
    CLogVota::GetInst().Loga("Eleitor foi suspenso e não confirmou nenhum voto");
    api::IBeep::GetInst().BeepSuspensao();                          // :253  IBeep slot 5 (name inferred)
    api::ISound::GetInst().Stop();                                  // :254  ISound slot 5
    // g_votosEleitor is NOT cleared here; it is discarded at the next StartState.
}

// ---------------------------------------------------------------------------------------------
// wasm func 7352 — vtable slot 3, srcloc :294. Observed executing.
bool CEleitorVotando::NeedChangeState()
{
    if (!m_estadoCargo) {
        CThreadEleitor::GetInst().StopTick(m_tickEleitorDemorando);
        return true;
    }
    if (!m_estadoCargo->NeedChangeState())
        return false;

    if (auto* proximo = m_estadoCargo->GetNextState()) {
        // transition inside the same cargo (e.g. CPedeProporcional -> CPedeNominal -> conferência)
        if (m_estadoCargo)
            m_estadoCargo->FinishState();
        m_estadoCargo = proximo;
        proximo->StartState();
        return false;
    }

    // sub-state returned nullptr: the current cargo/escolha is done
    if (m_vemDaInstrucaoAcessibilidade) {
        m_vemDaInstrucaoAcessibilidade = false;                     // instructions were not a cargo
    } else {
        auto& cargos = comum::CCargos::GetInst();
        if (!cargos.IsEnd()) {
            if (cargos.GetCurrent().GetQtdEscolhas() == g_numeroEscolha) {
                cargos.Next();
                g_numeroEscolha = 1;
            } else {
                ++g_numeroEscolha;
            }
        }
    }

    auto& cargos = comum::CCargos::GetInst();
    auto& beep = api::IBeep::GetInst();                             // srcloc :294
    const bool acabou = cargos.IsEnd();
    beep.Beep(1);                                                   // IBeep slot 2: one short beep
    if (acabou) {
        GravaVotos();          // func 4454: votes -> RDV, estado geral, next = CSincronismoEleitor
        return true;
    }
    InformaCargoAoOperador(cargos.GetCurrent());
    ChamaEstadoProximoCargo();
    return false;
}

// ---------------------------------------------------------------------------------------------
// wasm func 7370 — vtable slot 5. Observed executing.
void CEleitorVotando::FinishState()
{
    CThreadOperador::GetInst().m_textoCargo = " ";
    api::CApplicationContextStack::Remove(m_contexto);              // func 5553
}

// ---------------------------------------------------------------------------------------------
// wasm func 7337 — vtable slot 6 (message from the operator thread). name inferred
void CEleitorVotando::ProcessMessage(uebyte mensagem)
{
    switch (static_cast<EMensagemEleitor>(mensagem)) {
    case EMensagemEleitor::SuspensaoMesario:
        suspenderEleitor(EOrigemSuspensao::Mesario);
        return;
    case EMensagemEleitor::SuspensaoAutomatica:
        suspenderEleitor(EOrigemSuspensao::AutomaticaTreinamento);
        return;
    case EMensagemEleitor::ContinuaVotacao: {
        auto& thread = CThreadEleitor::GetInst();
        thread.StopTick(m_tickEleitorDemorando);
        thread.StartTick(m_tickEleitorDemorando);
        m_eleitorDemorando = false;
        return;
    }
    default:
        if (m_estadoCargo)
            m_estadoCargo->ProcessMessage(mensagem);
    }
}

// ---------------------------------------------------------------------------------------------
// wasm func 7345 — vtable slot 7 (a key is available). Observed executing.
void CEleitorVotando::ProcessInput()
{
    auto& thread = CThreadEleitor::GetInst();
    thread.StopTick(m_tickEleitorDemorando);                        // restart the inactivity timer
    thread.StartTick(m_tickEleitorDemorando);

    if (m_eleitorDemorando) {
        CThreadOperador::GetInst().EnviaMensagem({EMensagemOperador::EleitorVoltouADigitar}, 1);
        CLogVota::GetInst().Loga("Eleitor voltou a digitar enquanto estava sendo suspenso");
        m_eleitorDemorando = false;
    }
    if (m_estadoCargo)
        m_estadoCargo->ProcessInput();
}

// ---------------------------------------------------------------------------------------------
// wasm func 7349 — vtable slot 8. Observed executing.
void CEleitorVotando::ProcessTick(uebyte tick)
{
    if (tick == m_tickEleitorDemorando) {
        CThreadEleitor::GetInst().StopTick(m_tickEleitorDemorando);
        const bool nenhumVoto = comum::CCargos::GetInst().GetPosicao() == 0 && g_numeroEscolha == 1;
        CThreadOperador::GetInst().EnviaMensagem(
            {nenhumVoto ? EMensagemOperador::EleitorDemorandoSemVoto
                        : EMensagemOperador::EleitorDemorandoComVoto}, 1);
        m_eleitorDemorando = true;
        return;
    }
    if (m_estadoCargo)
        m_estadoCargo->ProcessTick(tick);
}

// ---------------------------------------------------------------------------------------------
// wasm func 4442 — srcloc :410/:427. Called for operator messages 2 and 3.
void CEleitorVotando::suspenderEleitor(const EOrigemSuspensao origem)
{
    auto& cargos = comum::CCargos::GetInst();
    auto& log = CLogVota::GetInst();
    if (origem == EOrigemSuspensao::AutomaticaTreinamento)
        log.Loga("Eleitor foi suspenso automaticamente no treinamento eleitor");
    else
        log.Loga("Eleitor foi suspenso pelo mesário");

    const auto info = comum::CConfiguracaoEleicao::GetInst().GetInformacaoEleicao();  // cfg +88
    comum::md::CVoto::ETipo tipo;

    if (cargos.GetPosicao() == 0 && g_numeroEscolha == 1) {
        // the voter did not confirm anything yet
        switch (info.GetFormaSuspensaoSemVoto()) {                  // cfg +88
        case 0: DescartaVotos(); return;
        case 1: tipo = comum::md::CVoto::ETipo::BrancoAposSuspensao; break;     // 5
        case 2: tipo = comum::md::CVoto::ETipo::NuloAposSuspensao; break;       // 6
        default:
            throw CUeVotaError(9312, "Forma suspensao sem voto invalida",
                               std::source_location::current());                // :410
        }
    } else {
        // at least one vote already confirmed
        switch (info.GetFormaSuspensaoComVoto()) {                  // cfg +92
        case 0: tipo = comum::md::CVoto::ETipo::BrancoAposSuspensao; break;
        case 1: tipo = comum::md::CVoto::ETipo::NuloAposSuspensao; break;
        case 2: DescartaVotos(); return;                            // even confirmed votes are dropped
        default:
            throw CUeVotaError(9313, "Forma suspensao com voto invalida",
                               std::source_location::current());                // :427
        }
    }

    // Fill every remaining escolha of every remaining cargo (inlined helper, name unknown)
    auto& candidaturas = comum::CCandidaturas::GetInst();           // func 521
    while (!cargos.IsEnd()) {
        const auto& cargo = cargos.GetCurrent();
        const bool temCandidato = candidaturas.ExisteCandidato(cargo.GetId());   // func 2271
        // RegistraVoto (func 4471) is inlined here: the first loop keeps its complete 5-way log
        // switch (on tipo - 5), the second one has it folded to LogaVotoCargoSemCandidatoSuspensao
        // (the compiler unswitched the loop on the "has candidates" test). The empty number is a
        // temporary std::string destroyed after each call.
        for (uebyte escolha = g_numeroEscolha; escolha <= cargo.GetQtdEscolhas(); ++escolha) {
            if (temCandidato || cargo.EhConsulta())
                RegistraVoto(cargo.GetId(), tipo, "");
            else
                RegistraVoto(cargo.GetId(), comum::md::CVoto::ETipo::NuloAposSuspensaoCargoSemCandidato, "");
        }
        g_numeroEscolha = 1;
        cargos.Next();
    }
    CLogVota::GetInst().Loga("Eleitor votou parcialmente e em seguida foi suspenso");
    GravaVotos();                                                   // func 4454
}

// ---------------------------------------------------------------------------------------------
// wasm func 4471 (no source file assigned by the analyzer). Called by
// CConfirmaVotoSemCandidato::ProcessInputAudio (tipo 8) and CConfirmaVotoEmCargo::ProcessInputAudio
// (func 11754, tipo = the state's +32). name inferred
void CEleitorVotando::RegistraVoto(TCargoID cargo, comum::md::CVoto::ETipo tipo, const std::string& numero)
{
    InsereVoto(cargo, comum::md::CVoto(tipo, numero));
    auto& log = CLogVota::GetInst();
    using T = comum::md::CVoto::ETipo;
    switch (tipo) {
    case T::BrancoAposSuspensao:                log.LogaVotoBrancoSuspensao(cargo); break;          // 5
    case T::NuloAposSuspensao:                  log.LogaVotoNuloSuspensao(cargo); break;            // 6
    case T::NuloCargoSemCandidato:              log.LogaVotoCargoSemCandidato(cargo); break;        // 8
    case T::NuloAposSuspensaoCargoSemCandidato: log.LogaVotoCargoSemCandidatoSuspensao(cargo); break; // 9
    default:                                    log.LogaVotoConfirmado(cargo); break;               // 1-4, 7
    }
}

// wasm func 4192. name inferred
void CEleitorVotando::InsereVoto(TCargoID cargo, const comum::md::CVoto& voto)
{
    g_votosEleitor.push_back({cargo, voto});
}

// =============================================================================================
// Functions emitted in this translation unit / attributed here, whose real owner is another class
// =============================================================================================

// wasm func 1785 — vota::CVotacaoStateAudio::CVotacaoStateAudio(uebyte flags). Real file:
// uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp (unit u08). Observed executing.
//
//   (member names as in unit u08's cvotacaostateaudio.h)
//   CVotacaoStateAudio::CVotacaoStateAudio(uebyte flags)
//       : comum::CAppState(flags)                 // shared_f224
//       , m_reservado{}                           // +16 unique_ptr (released through slot 1 in the dtor)
//       , m_esperaFimAudio{}                      // +20 shared_ptr<api::IEsperaAudio> (+24 ctrl)
//       , m_audioHabilitado(false)                // +11
//   {
//       m_tickRepeticao = CThreadEleitor::GetInst().CriaTick(2000);   // +12 (func 807: created stopped)
//       m_tickInicio    = CThreadEleitor::GetInst().CriaTick(1500);   // +13
//   }
//   NB: slot 12 (ProcessTickAudio) is pure in this base; CInstrucaoVotacaoAcessibilidade,
//   CConfirmaVotoSemCandidato and CConfirmaVotoEmCargo override it with a no-op (func 425).

// wasm func 6051 — merged body (wasm-opt merge-similar-functions) of the lazy singletons of two
// 28-byte CVotacaoStateAudio subclasses with flags 2:
//   func 3849  CPedeProporcional& CPedeProporcional::GetInst()  -> func6051(mutex 1838084, &s 1838108, vtable)
//   func 5921  CPedeMajoritario&  CPedeMajoritario::GetInst()   -> func6051(mutex 1838316, &s 1838340, vtable)
//   template <class T> T& GetInstancia(std::mutex& m, std::unique_ptr<T>& s) {
//       if (!s) s = std::make_unique<T>();    // CVotacaoStateAudio(2) + vtable
//       m.unlock();                           // (lock elided in the single-threaded build)
//       return *s;
//   }

// wasm func 7416 — atexit destructor of CEleitorVotando's singleton storage (unique_ptr reset).
// wasm func 3783 — std::vector<CCargos::SItem /*8 bytes: {int eleicao; uebyte cargo}*/>::
//                  __assign_with_size(first, last, n): libc++ instantiation used by
//                  CCargos::FiltraPorAbrangencia (inlined in StartState) and comum_f3784.

// wasm func 2302 — merged body of the CLogVota "vote logged for cargo" helpers below
// (merge-similar-functions: only the format string differs). Real owner: vota::CLogVota
// (uenux2/src/app/vota/log/clogvota.cpp, path inferred); the first argument is the CLogVota.
//   void CLogVota::LogaXxx(const comum::TCargoID cargo) {
//       Loga(std::format(FMT, comum::CConfiguracaoEleicao::GetInst().GetCargo(cargo).GetNome()));
//   }
// func 4537 LogaVotoConfirmado                  "Voto confirmado para [{}]"
// func 4546 LogaVotoBrancoSuspensao             "Atribuido voto branco por suspensão [{}]"
// func 4541 LogaVotoNuloSuspensao               "Atribuido voto nulo por suspensão [{}]"
// func 4542 LogaVotoCargoSemCandidato           "Atribuido voto para cargo sem candidato [{}]"
// func 3272 LogaVotoCargoSemCandidatoSuspensao  "Atribuido voto para cargo sem candidato por suspensão [{}]"
// (all names inferred; the log file only receives the cargo name, never the vote content)

}  // namespace vota
