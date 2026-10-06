// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original (path inferred): uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.h
//
// "Nome do eleitor" (screen "telaNomeEleitor"): after the typed título/CPF was found in the roll, the MT
// shows the voter's name, identity, sequential number and seção (and his photo on the MT LCD) so the
// mesário can confirm he is the person present. CONFIRMA chooses how the voter will be released:
// fingerprint (CControlaReconhecimento), birth year (CPedeAnoNascimentoSemBiometria) or directly.
//
// RTTI: comum::CAppState <- vota::CNomeEleitor (typeinfo @1591324, vtable @1591256)
//   slot 0 icf 244  1 icf 387  2 StartState (10501)  7 ProcessInput (10500)
#pragma once

#include <memory>

#include "comum/cappstate.h"
#include "api/gui/cinteractiveform.h"

namespace vota {

class CNomeEleitor final : public comum::CAppState {
public:
    /// wasm func 5401 (unit u17). Lazy singleton (@..., 20 bytes): CAppState(2), m_form "telaNomeEleitor":
    /// (1,1) name "{:2}", (1,2) typed identity (func 10586), "Seq:" + CEleitorDadoSequencial "{:04}",
    /// "Seção:" + CEleitorDadoSecao, TTE text (slot 4113), "CORRIGE: cancelar", "CONFIRMA: prosseguir".
    static CNomeEleitor& GetInst();

    void StartState() override;                              // slot 2 (func 10501)
    void ProcessInput() override;                            // slot 7 (func 10500)

private:
    void NavegaBiometrica();                                 // srcloc line 145 (inlined twice)
    void NavegaAnoNascimento();                              // srcloc line 156 (inlined)
    void HabilitaEleitorSemBiometria();                      // func 5400, name inferred

    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +12/+16
};

}  // namespace vota
