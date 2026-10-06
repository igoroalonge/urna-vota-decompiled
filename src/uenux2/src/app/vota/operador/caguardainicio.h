// Reconstructed from vota_web_wasm.wasm (unit u39; constructor + GetInst by unit u27).
// Original (path inferred): uenux2/src/app/vota/operador/caguardainicio.h
// (u27 filed the constructor under this path; the tools put GetInst, func 5343, under cthreadoperador.cpp).
//
// "Aguarda início" = first state of the operator thread (terminal do mesário, MT). The MT shows the
// application name/version, "Siga as instruções na tela do eleitor" and a clock, while the voter thread runs
// the start-of-day procedure (zerésima, keypad test...). It leaves when the voter thread posts a message:
//    7  -> comum::CRegistrarMesarios  (registration of the mesários before voting; CDefineRotaPreVotacao,
//                                      CReinicioComparecimentoMesario)
//    8  -> CPedeIdentidade            (voting started: CInicioVotacao, func 5972) + log
//   10  -> CFimAquisicaoVotos         (restart during the closing: CFinalizaAquisicao, func 12113)
//
// RTTI: api::CState <- comum::CAppState <- vota::CAguardaInicio (typeinfo @1601076, vtable @1601040)
//   [0] ICF 244 dtor  [1] ICF 387 deleting  [2] StartState 10220  [3] 7480  [4] 1661
//   [5] FinishState 10219  [6] ProcessMessage 10217  [7] nop 218  [8] ProcessTick 10218
//
// WEB BUILD: dead code - CThreadOperador::Run (func 10204) never runs in the simulator (unit u10 §2).
#pragma once

#include <cstdint>
#include <memory>

#include "api/gui/iform.h"
#include "comum/cappstate.h"

namespace vota {

using uebyte = std::uint8_t;

class CAguardaInicio final : public comum::CAppState {
public:
    /// wasm func 5343 (unit u27): lazy singleton @1911624 (mutex residue @1911600), 24 bytes,
    /// CAppState(5 = messages + ticks). Callers: CThreadOperador::Run (10204) and
    /// CControladorRegistraMesariosVota::GetEstadoAposRegistroInicial (10794).
    static CAguardaInicio& GetInst();

    void StartState() override;                        // [2] wasm func 10220
    void FinishState() override;                       // [5] wasm func 10219
    void ProcessMessage(uebyte mensagem) override;     // [6] wasm func 10217
    void ProcessTick(uebyte tick) override;            // [8] wasm func 10218

private:
    CAguardaInicio();                                  // inlined into 5343 (see u27-foreign-fragments.cpp)

    // +0 vptr, +4 m_proximoEstado, +8..+10 CAppState flags
    std::shared_ptr<api::IForm<api::IScreenMT>> m_form;   // +12 (+16 ctrl) non-interactive MT form
    uebyte m_tick;                                        // +20 500 ms clock refresh (CThreadOperador tick)
};

}  // namespace vota
