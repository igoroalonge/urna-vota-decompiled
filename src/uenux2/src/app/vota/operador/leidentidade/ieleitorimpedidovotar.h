// Reconstructed from vota_web_wasm.wasm (unit u27).
// Original file UNKNOWN - path inferred: uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.h
// (the tools attributed its constructor to celeitorencontrado.cpp, the main user; the class is the base of
// every "this voter cannot vote here" screen reached from CProcuraEleitor / CEleitorEncontrado).
//
// RTTI: comum::CAppState <- vota::IEleitorImpedidoVotar (typeinfo @1589184, vtable @1589144, 10 slots)
//   [0] dtor 1257 (merged body 2902)  [1] deleting dtor 1689 (shared by all subclasses)
//   [2] StartState 10625 (shows the form + the voter photo, u05)   [7] ProcessInput 10624   [9] hook, no-op
// Subclasses (all 24 bytes, same constructor, only the two text lines and the key differ):
//   CEleitorNaoEncontrado                   tipo 0 "CONFIRMA: retornar"  texts slots 3867/3868   (vtable @1588456)
//   CEleitorOptouPorVotarEmTransito         tipo 0                       texts slots 3876/3877   (@1588516)
//   CEleitorImpedidoJustificar              tipo 1 "CORRIGE: retornar"   texts slots 3883/3884   (@1588576)
//   CEleitorNaoTemIdadeMinima               tipo 1                       texts slots 3889/3890   (@1588636)
//   CEleitorNaoPossuiCargosParaVotar        tipo 1                       texts slots 3895/3896   (@1588696)
//   CEleitorImpedidoJustificarVotoTransito  tipo 1                       texts slots 3914/3915   (@1588884)
// NOT a subclass (RTTI: it derives directly from comum::CAppState, 20 bytes, own vtable @1589596):
//   CEleitorImpedidoJustificarVotoCPF       (ProcessInput 10604 = shared body 2295, CORRIGE)
#pragma once

#include <functional>
#include <memory>
#include <string>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

class IEleitorImpedidoVotar : public comum::CAppState {
public:
    /// Which key leaves the screen (and where it is drawn). Values attested, names inferred.
    enum ETipoRetorno : int { RETORNA_CONFIRMA = 0, RETORNA_CORRIGE = 1 };

    /// wasm func 1905 (tools: vota_f1905)
    IEleitorImpedidoVotar(std::function<std::string()> linha2, std::function<std::string()> linha3,
                          ETipoRetorno tipo);
    ~IEleitorImpedidoVotar() override;                 // slot 0 (func 1257 -> merged body 2902), slot 1 (1689)

    void StartState() override;                        // slot 2 (func 10625, other unit)
    void ProcessInput() override;                      // slot 7 (func 10624, other unit)
    virtual void AoRetornar() {}                       // slot 9 (no-op; CEleitorNaoEncontrado: 10663)   name inferred

protected:
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +12/+16
    ETipoRetorno m_tipo;                                                            // +20
};

// The concrete screens only pass their texts (lambdas / functions in table slots) to the base.
#define VOTA_ELEITOR_IMPEDIDO(Classe)                                            \
    class Classe final : public IEleitorImpedidoVotar {                         \
    public:                                                                     \
        static Classe& GetInst();                                               \
    private:                                                                    \
        Classe();                                                               \
    };

VOTA_ELEITOR_IMPEDIDO(CEleitorNaoEncontrado)                  // GetInst = wasm func 5424 (u27)
VOTA_ELEITOR_IMPEDIDO(CEleitorOptouPorVotarEmTransito)        // GetInst inlined into 10635
VOTA_ELEITOR_IMPEDIDO(CEleitorImpedidoJustificar)             // GetInst inlined into 10635
VOTA_ELEITOR_IMPEDIDO(CEleitorNaoTemIdadeMinima)              // GetInst inlined into 10635
VOTA_ELEITOR_IMPEDIDO(CEleitorNaoPossuiCargosParaVotar)       // GetInst inlined into 10635
VOTA_ELEITOR_IMPEDIDO(CEleitorImpedidoJustificarVotoTransito) // GetInst inlined into 10635

}  // namespace vota
