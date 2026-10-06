// Reconstructed from vota_web_wasm.wasm (unit u27; constructor by unit u17).
// Original (path inferred from cpedeidentidade.cpp, attested by srclocs :62/:63):
// uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.h
//
// "Pede identidade": the idle screen of the mesário's terminal between two voters. It asks for the
// next voter's identity number ("Digite o Título ou o CPF"), shows how many voters have voted
// ("0003/0150"), the audio indicator and, every second, "CORRIGE: outras opções" while nothing is typed.
// Its timers also block the vote at the end of the day and schedule the random booth inspections.
//
// RTTI: api::CState <- comum::CAppState <- vota::CPedeIdentidade (vtable: slot 2 StartState 10680,
//   5 FinishState 10678, 7 ProcessInput 10677, 8 ProcessTick 10676; others CAppState defaults)
#pragma once

#include <memory>
#include <string>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

class CPedeIdentidade final : public comum::CAppState {
public:
    /// wasm func 652 (unit u17; tools "api::CTextSource::CTextSource@652"). Lazy singleton @1905336
    /// (mutex @1905312), 32 bytes; constructor inlined (see cpedeidentidade.cpp).
    static CPedeIdentidade& GetInst();

    void StartState() override;                    // slot 2 (func 10680, srcloc :62, :63)
    void FinishState() override;                   // slot 5 (func 10678)
    void ProcessInput() override;                  // slot 7 (func 10677)
    void ProcessTick(uebyte tick) override;        // slot 8 (func 10676)

private:
    CPedeIdentidade();

    comum::CAppState* ValidaEntrada();             // inlined into ProcessInput, name inferred

    // +0 vptr, +4 m_proximoEstado, +8/+9/+10 CAppState flags (6 = keys + ticks)
    uebyte m_tickBloqueio;                         // +11  60 s: VotacaoBloqueadaPorHorario
    uebyte m_tickInspecao;                         // +12  5 s: time for a booth inspection?
    uebyte m_tickStatus;                           // +13  1 s: refresh of the status line
    std::shared_ptr<std::string> m_textoStatus;    // +16  " " or "CORRIGE: outras opções"
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +24
};

}  // namespace vota
