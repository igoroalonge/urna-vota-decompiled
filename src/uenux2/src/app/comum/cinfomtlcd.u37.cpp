// uenux2/src/app/comum/cinfomtlcd.cpp (attested file) -- FRAGMENT written by unit u37.
// Class and layout: cinfomtlcd.h (unit u22).
#include <algorithm>
#include <mutex>

#include "comum/cinfomtlcd.h"

namespace comum {

// wasm func 5903 (tools: vota_f5903)                                  name inferred (as used by u17/u27)
// Callers: vota::CPedeIdentidade::StartState (10680) and vota::CMostraEleitorVotando::StartState (10427),
// i.e. every time the operator terminal goes back to "type the voter's identifier": the battery icon comes
// back on the microterminal LCD after a voter photo was shown there (MostrarImagemNoLCD detaches it).
// IObservable::Attach is inlined with an "only once" test and an immediate notification:
void CInfoMTLCD::ExibeBateria()
{
    std::lock_guard trava(m_bateria.m_mutex);                                        // +28 (unlock stub 150)
    auto& observadores = m_bateria.m_observadores;                                   // +16 begin / +20 end
    if (std::find(observadores.begin(), observadores.end(), this) == observadores.end()) {   // shared_f736
        observadores.push_back(this);                                                // wasm 543
        Update(m_bateria.m_valor);                                                   // slot 2 (wasm 11653), icon at +8
    }
}

} // namespace comum
