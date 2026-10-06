// Reconstructed from vota_web_wasm.wasm (unit u39; constructor by u17, ProcessInput by u19).
// Original (path inferred by u17): uenux2/src/app/vota/operador/aguardaeleitor/cperguntacodigosuspensao.h
//
// "Pergunta código de suspensão": the mesário wants to suspend the vote of a voter who is taking too long
// (CPerguntaEleitorVotando -> CORRIGE). The MT asks for the mesário's título eleitoral ("Informe seu título
// para / suspender a votação"); a valid título suspends the voter (message 2 to the voter thread, see
// ProcessInput 10420); CORRIGE aborts ("Mesário abortou processo de suspensão", message 4 = resume voting).
// While the question is on screen the voter may still finish or resume: messages 2, 5 and 13 from the
// voter thread cancel the question (ProcessMessage).
//
// RTTI: comum::CAppState <- vota::CPerguntaCodigoSuspensao (typeinfo @1593164, vtable @1593128)
//   [0] ICF 448  [1] ICF 765  [2] StartState 10421  [3] 7480  [4] 1661  [5] nop
//   [6] ProcessMessage 10419  [7] ProcessInput 10420 (u19)  [8] nop
// Lazy singleton @1909344 (mutex @1909320); GetInst + ctor inlined into CPerguntaEleitorVotando::ProcessInput.
//
// WEB BUILD: dead code (operator thread not run).
#pragma once

#include <cstdint>
#include <memory>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

using uebyte = std::uint8_t;

class CPerguntaCodigoSuspensao final : public comum::CAppState {
public:
    static CPerguntaCodigoSuspensao& GetInst();

    void StartState() override;                        // [2] wasm func 10421
    void ProcessMessage(uebyte mensagem) override;     // [6] wasm func 10419
    void ProcessInput() override;                      // [7] wasm func 10420 (unit u19)

private:
    CPerguntaCodigoSuspensao();                        // CAppState(3 = messages + keys), see u17-foreign-fragments.cpp

    using TFormMT = std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>>;
    TFormMT m_form;                // +12 "Informe seu título para / suspender a votação" + 12-digit input
    TFormMT m_formInvalido;        // +20 "Título inválido para / suspender a votação"
    bool m_suspensaoEnviada;    // +28 set by ProcessInput after a VALID título (message 2 posted to the voter
                                   //     thread); while set, keys are ignored until the voter thread answers.
                                   //     Cleared by StartState and by messages 2/13. (u19: m_suspensaoEnviada; u17: m_tentativas ?)
                                   //     name inferred; sizeof 32
};

}  // namespace vota
