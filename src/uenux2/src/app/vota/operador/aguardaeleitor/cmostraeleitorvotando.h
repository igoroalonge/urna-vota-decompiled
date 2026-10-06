// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original (path inferred): uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.h
//
// "Mostra eleitor votando": the operator-thread state shown on the mesário's terminal (MT, 4x40 LCD)
// while a released voter is at the urna. It shows who is voting (name / identity / sequencial / seção)
// and the cargo being voted ("VOTANDO PARA: ..."), ignores the mesário's keys and reacts to the
// messages the voter thread (CEleitorVotando & co., unit u06) posts to CThreadOperador.
//
// RTTI: api::CState <- comum::CAppState <- vota::CMostraEleitorVotando (typeinfo @1593092, vtable @1593008)
//   slot 0 icf 448 (dtor)  1 icf 765 (deleting)  2 StartState (10427)  3 CAppState::NeedChangeState (7480)
//   4 GetNextState (1661)  5 FinishState = no-op (218)  6 ProcessMessage (10425)  7 ProcessInput (10426)
//   8 ProcessTick = no-op (425)
#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "comum/cappstate.h"
#include "api/gui/iform.h"                // api::IForm<api::IScreenMT>

namespace vota {

using uebyte = std::uint8_t;

/// Messages the voter thread posts to CThreadOperador (see src/uenux2/src/app/vota/eleitor/celeitorvotando.h,
/// which declares 1..9; 13 is posted by CSincronismoEleitor::StartState, func 7178). Names inferred.
enum class EMensagemOperadorRecebida : uebyte {
    FimVotoEleitor = 1,            // handled by CSincronismoOperador
    EleitorNaoVotou = 2,
    EleitorDemorandoSemVoto = 3,
    EleitorDemorandoComVoto = 4,
    EleitorVoltouADigitar = 5,
    EleitorIniciouVotacao = 6,
    AtualizaCargoAtual = 9,
    SincronizaVoto = 13,           // the voter thread waits in CSincronismoEleitor for the operator to record the vote
};

class CMostraEleitorVotando final : public comum::CAppState {
public:
    /// wasm func 1150 (unit u17; the tools call it "api::CTextSource::CTextSource@1150"). Lazy singleton
    /// (@1909316, 28 bytes): CAppState(3 = messages + keys), m_textoCargo = make_shared<std::string>(),
    /// m_form = non-interactive IForm<IScreenMT> (func 1694) with: LED on; line 1 "TREINAMENTO DE ELEITORES"
    /// (voter-training) or the voter's name (CEleitorDadoNomeParaUrna, "{:35}"); line 2 the typed identity
    /// (func 10586) + "Seq:" + sequencial "{:04}"; line 3 TTE text + "Seção:" + seção "{:04}";
    /// line 4 the cargo text (CDataText<CTextSource>(m_textoCargo)).
    static CMostraEleitorVotando& GetInst();

    void StartState() override;                              // slot 2 (func 10427)
    void ProcessMessage(uebyte mensagem) override;           // slot 6 (func 10425)
    void ProcessInput() override;                            // slot 7 (func 10426) (srcloc line 64)

private:
    void SalvaHabilitacaoEleitor();                          // inlined into slot 6 (srcloc lines 127, 159)

    std::shared_ptr<std::string>                 m_textoCargo;   // +12/+16  "VOTANDO PARA: ..."
    std::shared_ptr<api::IForm<api::IScreenMT>>  m_form;         // +20/+24
};

}  // namespace vota
