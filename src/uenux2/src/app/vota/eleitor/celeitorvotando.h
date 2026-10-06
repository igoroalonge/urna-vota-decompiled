// Reconstructed from vota_web_wasm.wasm (unit u06). Original: uenux2/src/app/vota/eleitor/celeitorvotando.h
// (path inferred from celeitorvotando.cpp, which is attested by std::source_location records).
//
// CEleitorVotando ("voter voting") is the top-level state of the voter thread (CThreadEleitor) while
// one voter is at the urna. It owns nothing but drives a *sub-state machine*: one sub-state per
// cargo/escolha (CPedeMajoritario, CPedeProporcional, CConfirmaVotoSemCandidato, the confirmation
// states...), stored in m_estadoCargo. When the sub-state finishes a cargo (GetNextState() == nullptr)
// CEleitorVotando advances comum::CCargos and creates the sub-state of the next cargo. When the list
// of cargos is exhausted it hands the collected votes to the RDV (wasm func 4454) and moves to
// CSincronismoEleitor.
//
// RTTI:  api::CState <- comum::CAppState <- vota::CEleitorVotando          (typeinfo @1533316)
//        vtable @1533152 (9 slots, see celeitorvotando.cpp)
#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "comum/cappstate.h"                    // comum::CAppState (path inferred)
#include "api/gui/capplicationcontextstack.h"   // api::CApplicationContext
#include "comum/dados/md/rdv/cvoto.h"           // comum::md::CVoto (unit u22)

namespace comum::md { class CCargo; }

namespace vota {

using uebyte = std::uint8_t;
using comum::TCargoID;                 // uebyte

// Origin of a suspension (message sent by the operator thread). name inferred for the values;
// the enum name is attested by the srcloc signature suspenderEleitor(const EOrigemSuspensao).
enum class EOrigemSuspensao : int {
    Mesario = 0,                        // "Eleitor foi suspenso pelo mesário"
    AutomaticaTreinamento = 1,          // "Eleitor foi suspenso automaticamente no treinamento eleitor"
};

// Messages the voter thread posts to the operator thread (CThreadOperador, priority 1).
// Values from the senders below and from the receiver vota::CMostraEleitorVotando::ProcessMessage
// (wasm func 10425). Enum/enumerator names inferred.
enum class EMensagemOperador : short {
    FimVotoEleitor = 1,                 // CFimVotoEleitor::StartState ("eleitor votou")
    EleitorNaoVotou = 2,                // DescartaVotos -> operator shows CEleitorVotouNaoVotou
    EleitorDemorandoSemVoto = 3,        // inactivity tick, nothing confirmed yet -> CEleitorDemorando
    EleitorDemorandoComVoto = 4,        // inactivity tick, some votes confirmed    -> CEleitorDemorando
    EleitorVoltouADigitar = 5,          // key pressed after the "demorando" alert
    EleitorIniciouVotacao = 6,          // StartState (operator saves the habilitação)
    AtualizaCargoAtual = 9,             // text in CThreadOperador +108 ("VOTANDO PARA: ...")
};

// Messages the voter thread receives (CMessageEleitor) and CEleitorVotando handles itself.
// (0/6 stop the thread and 8/9/10 set the audio mode: handled in CThreadEleitor::Processar, func 4349,
// by its inlined ProcessarMensagens.)
enum class EMensagemEleitor : uebyte {  // name inferred
    SuspensaoMesario = 2,
    SuspensaoAutomatica = 3,
    ContinuaVotacao = 4,                // operator dismissed the "eleitor demorando" alert
};

// ---------------------------------------------------------------------------------------------
// Process-wide data shared by CEleitorVotando and the per-cargo states (names inferred).
// ---------------------------------------------------------------------------------------------

/// Number currently typed by the voter for the current cargo (read by CPede*, CConfirma*,
/// CVotacaoStateAudio's audio templates and by the web adapter's votaGetStateJson, func 5500).
extern std::string g_votoDigitado;                                         // @1833288 (12 bytes)

/// Votes confirmed by the current voter, in order: (cargo, CVoto{tipo, numero}). 20-byte entries.
/// Cleared at StartState; consumed by CEleitorVotando::GravaVotos (func 4454, unit u22);
/// CPedeMajoritario scans it to detect a repeated candidate (CMajoritarioRepetido).
extern std::vector<std::pair<TCargoID, comum::md::CVoto>> g_votosEleitor;  // @1833300

/// 1-based index of the escolha (seat) inside the current cargo, e.g. 1st/2nd Senate seat.
/// Also read by DS_NomeCargoNeutroComEscolha and CPreShowProgressBar. Initial value 1.
extern uebyte g_numeroEscolha;                                             // @1536340

class CEleitorVotando final : public comum::CAppState {
public:
    /// Lazy singleton (wasm func 3229, unit u37; its constructor is inlined there):
    ///   CAppState(7 = messages|keyboard|ticks), m_estadoCargo = nullptr, flags = false,
    ///   m_contexto = CApplicationContext(11, "Erro inesperado durante a votação",
    ///                "O voto do eleitor NÃO foi registrado",
    ///                "Ocorreu um erro enquanto o eleitor registrava suas escolhas."),
    ///   m_tickEleitorDemorando = CThreadEleitor::GetInst().CriaTick(timeout * 1000) with
    ///   timeout = EhTreinamentoEleitor() ? cfg[+172] : cfg[+168] (seconds, CConfiguracaoEleicao).
    static CEleitorVotando& GetInst();

    ~CEleitorVotando() override;                            // slot 0 (func 2483), slot 1 (func 7385)

    void StartState() override;                             // slot 2 (func 7377)
    bool NeedChangeState() override;                        // slot 3 (func 7352)
    // slot 4 GetNextState(): inherited (returns m_proximoEstado)
    void FinishState() override;                            // slot 5 (func 7370)
    void ProcessMessage(uebyte mensagem) override;          // slot 6 (func 7337) name inferred
    void ProcessInput() override;                           // slot 7 (func 7345)
    void ProcessTick(uebyte tick) override;                 // slot 8 (func 7349)

    /// Stores one vote of the current voter and logs it (func 4471; `this` is unused, the callers
    /// still call GetInst() first). name inferred
    void RegistraVoto(TCargoID cargo, comum::md::CVoto::ETipo tipo, const std::string& numero);

private:
    void IniciaCiclo();                                     // inlined into StartState (srcloc 125,131)
    void ChamaEstadoProximoCargo();                         // func 4459 (srcloc 229)
    void DescartaVotos();                                   // func 4449 (srcloc 253,254)
    void suspenderEleitor(const EOrigemSuspensao origem);   // func 4442 (srcloc 410,427)
    void GravaVotos();                                      // func 4454, unit u22 (name inferred)

    static void InsereVoto(TCargoID cargo, const comum::md::CVoto& voto);   // func 4192 name inferred

    // layout (72 bytes; CAppState part: +0 vptr, +4 m_proximoEstado, +8/+9/+10 flags)
    comum::CAppState* m_estadoCargo = nullptr;      // +12  current per-cargo sub-state (not owned)
    uebyte m_tickEleitorDemorando;                  // +16  inactivity tick id (CThreadEleitor)
    bool m_eleitorDemorando = false;                // +17  "demorando" alert sent to the operator
    bool m_vemDaInstrucaoAcessibilidade = false;    // +18  don't advance the cargo once
    api::CApplicationContext m_contexto;            // +20  (52 bytes) error context pushed while voting
};

}  // namespace vota
