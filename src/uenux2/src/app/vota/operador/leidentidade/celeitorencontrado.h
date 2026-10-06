// Reconstructed from vota_web_wasm.wasm (unit u27).
// Original (path inferred from celeitorencontrado.cpp, attested by srcloc :102):
// uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.h
//
// "Eleitor encontrado": the typed identity was found in the section roll (CProcuraEleitor positioned
// CEleitores on the voter). This state decides, without drawing anything, which screen comes next:
// the voter's impediments (impedimentos), whether he already voted, whether he has no cargo to vote in
// this urna (voter in transit), or the normal path (CNomeEleitor: name + photo, then habilitação).
//
// RTTI: comum::CAppState <- vota::CEleitorEncontrado (vtable @1588944; slot 2 StartState = func 10635).
// 12 bytes, CAppState(0): receives no messages, keys or ticks - it only transits.
#pragma once

#include "comum/cappstate.h"

namespace vota {

class CEleitorEncontrado final : public comum::CAppState {
public:
    /// Lazy singleton @1905616 (mutex @1905592); GetInst + constructor inlined into
    /// CProcuraEleitor::StartState (func 10631).
    static CEleitorEncontrado& GetInst();

    void StartState() override;           // slot 2 (func 10635, srcloc :102)

private:
    CEleitorEncontrado() : comum::CAppState(0) {}
    comum::CAppState* EstadoEleitorImpedido(const comum::CEleitorDetalhe& eleitor);   // inlined, name inferred
};

}  // namespace vota
