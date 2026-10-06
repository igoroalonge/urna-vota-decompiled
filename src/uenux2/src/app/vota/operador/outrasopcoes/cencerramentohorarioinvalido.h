// Reconstructed from vota_web_wasm.wasm (unit u27).
// Original (path inferred from cencerramentohorarioinvalido.cpp, attested by srclocs :60/:73):
// uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.h
//
// "Encerramento em horário inválido": the mesário chose "2-Encerrar votação" before the configured
// earliest closing time (HorariosUrna.encerramentoVotacao, e.g. 17:00). The MT shows
//     hh:mm (33,1)
//     Encerramento de votação inválido      (centred on (1,2))
//     Antes do horário                      (centred on (1,3))
//     CORRIGE: retornar
// In demonstration mode or in mesário training the state ALSO moves the urna clock to
// encerramentoVotacao - 10 s (10 seconds BEFORE the earliest closing time: 16:59:50 for 17:00:00), so
// that the closing can be rehearsed 10 seconds later (see StartState).
//
// RTTI: comum::CAppState <- vota::CEncerramentoHorarioInvalido (vtable @1587772; slot 2 StartState 10710,
//   slot 7 ProcessInput 10709). 20 bytes: CAppState(2), +12 m_form.
#pragma once

#include <memory>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

class CEncerramentoHorarioInvalido final : public comum::CAppState {
public:
    /// Lazy singleton @1905224 (mutex @1905200); GetInst + constructor inlined into
    /// CIniciaFinalizacao::StartState (func 10706).
    static CEncerramentoHorarioInvalido& GetInst();

    void StartState() override;                    // slot 2 (func 10710, srcloc :60, :73)
    void ProcessInput() override;                  // slot 7 (func 10709 -> shared body 2295)

private:
    CEncerramentoHorarioInvalido();
    void AvancaRelogioParaEncerramento();          // inlined into StartState, name inferred

    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +12/+16
};

/// wasm func 2295 (tools: api_f2295; attributed to this file). Body shared by five ProcessInput
/// implementations that only leave the screen: "if the key read is `tecla`, go back to CPedeIdentidade".
///   CEncerramentoHorarioInvalido (10709, CORRIGE), CHorarioVotacaoTerminou (10757, CORRIGE),
///   CIdentidadeInvalida (10667, CORRIGE), CEleitorImpedidoJustificarVotoCPF (10604, CORRIGE),
///   CTentativaCapturaDigitalEsgotada (10461, CONFIRMA).
/// The third argument of the wasm function is the srcloc record cinteractiveform.h:57 of the caller.
void RetornaParaPedeIdentidadeSeTecla(comum::CAppState& estado,
                                      api::CInteractiveForm<api::IScreenMT, api::IInputMT>& form,
                                      api::EInputResult tecla);                    // name inferred

}  // namespace vota
