// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original (path inferred): uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.h
//
// "Suspensão automática do eleitor": in voter-training mode (treinamento de eleitores) an inactive
// voter is suspended automatically. When the voter thread reports inactivity (message 3/4) the operator
// goes to CEleitorDemorando, whose StartState (func 10440, unit u17) switches to this state when
// comum::EhTreinamentoEleitor(). It counts down 10 seconds on the MT ("Suspensão automática em N
// segundos...") beeping every second; at zero it tells the voter thread to suspend the voter
// (message 3, see CEleitorVotando::suspenderEleitor, unit u06). CORRIGE aborts the countdown.
//
// RTTI: comum::CAppState <- vota::CSuspensaoAutomaticaEleitor (typeinfo @1593308, vtable @1593272)
//   slot 0 dtor (2732)  1 deleting (10411)  2 StartState (10409)  3 NeedChangeState (7480)
//   4 GetNextState (1661)  5 FinishState (10408)  6 ProcessMessage (10404)  7 ProcessInput (10407)
//   8 ProcessTick (10405)
#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "comum/cappstate.h"
#include "api/gui/cinteractiveform.h"

namespace vota {

using uebyte = std::uint8_t;

class CSuspensaoAutomaticaEleitor final : public comum::CAppState {
public:
    /// Lazy singleton (@1909400, 56 bytes). GetInst + constructor are inlined into
    /// CEleitorDemorando::StartState (func 10440, unit u17):
    ///   CAppState(7 = messages + keys + ticks); m_votouParcialmente = false;
    ///   m_tick = CThreadOperador::GetInst().CriaTick(1000); m_segundosRestantes = 10s;
    ///   m_suspensaoEnviada = false; m_textoSituacao = make_shared<string>("votou/não votou");
    ///   m_textoContagem = make_shared<string>("contagem regressiva");
    ///   m_form = CInteractiveForm<IScreenMT, IInputMT>: LED on, (1,1) m_textoContagem "%s",
    ///   (1,2) m_textoSituacao "%s", (40,3) right-aligned "CORRIGE: Cancelar", input control.
    ///   After GetInst the caller copies its own flag: m_votouParcialmente = CEleitorDemorando::m_votouParcialmente.
    /// The static unique_ptr is reset at exit by func 10414.
    static CSuspensaoAutomaticaEleitor& GetInst();

    ~CSuspensaoAutomaticaEleitor() override;                 // slot 0 (2732) / slot 1 (10411)
    void StartState() override;                              // slot 2 (10409)
    void FinishState() override;                             // slot 5 (10408)
    void ProcessMessage(uebyte mensagem) override;           // slot 6 (10404)
    void ProcessInput() override;                            // slot 7 (10407)
    void ProcessTick(uebyte tick) override;                  // slot 8 (10405)

    bool m_votouParcialmente = false;                        // +11 (written by CEleitorDemorando)

private:
    void AtualizaTextoContagem();                            // func 3612, name inferred
    static void DisparaSinalizacaoSonora();                  // func 5392 (lambda :89 = func 10410)

    uebyte                           m_tick;                 // +12  1000 ms tick of CThreadOperador
    std::chrono::seconds             m_segundosRestantes{10};// +16  (int64)
    bool                             m_suspensaoEnviada = false;   // +24
    std::shared_ptr<std::string>     m_textoSituacao;        // +28/+32 "Votou parcialmente" / "Não votou"
    std::shared_ptr<std::string>     m_textoContagem;        // +36/+40 "Suspensão automática em N segundos..."
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +44/+48
};

}  // namespace vota
