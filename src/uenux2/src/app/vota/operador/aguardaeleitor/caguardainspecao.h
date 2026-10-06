// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/operador/aguardaeleitor/caguardainspecao.h
// (include path already used by cpedeidentidade.cpp; the class sits with the other "waiting for the
// voter terminal" states).
//
// "Aguarda inspeção": periodic inspection of the voting booth ("inspeção da cabina e da urna"). Between
// voters, CPedeIdentidade::ProcessTick (u10/u17) notices that the drawn inspection time (now + 60..90 min)
// has passed, posts message 12 to the voter thread (which shows CInspecionaUrna: "Por favor, inspecione
// cabina e urna.") and enters this state. The MT shows "Inspecione cabina e urna / Instruções no terminal
// do eleitor" and waits for message 11 (the mesário confirmed on the voter keypad), then goes to
// CConfirmaInspecionada ("Inspeção completa / CONFIRMA: continuar a votação").
//
// RTTI: comum::CAppState <- vota::CAguardaInspecao (typeinfo @1590768, vtable @1590732)
//   [0] ICF 244  [1] ICF 387  [2] ICF 1070 {m_proximoEstado = this; m_form->Show();}  [3] 7480  [4] 1661
//   [5] nop  [6] ProcessMessage 10530  [7] nop  [8] nop
// Lazy singleton @1908552 (mutex @1908528); GetInst + constructor are inlined into CPedeIdentidade::ProcessTick:
// CAppState(1 = messages only), non-interactive MT form (func 1694): buzzer (51, 10), clock {33,1},
// (1,2) "Inspecione cabina e urna", (1,3) "Instruções no terminal do eleitor".
//
// WEB BUILD: dead code (operator thread not run).
#pragma once

#include <cstdint>
#include <memory>

#include "api/gui/iform.h"
#include "comum/cappstate.h"

namespace vota {

using uebyte = std::uint8_t;

class CAguardaInspecao final : public comum::CAppState {
public:
    static CAguardaInspecao& GetInst();                // inlined into CPedeIdentidade::ProcessTick

    void StartState() override;                        // [2] ICF body 1070 (shared by 12 classes)
    void ProcessMessage(uebyte mensagem) override;     // [6] wasm func 10530

private:
    CAguardaInspecao();

    std::shared_ptr<api::IForm<api::IScreenMT>> m_form;   // +12 (+16 ctrl); sizeof 20
};

}  // namespace vota
