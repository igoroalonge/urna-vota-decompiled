// uenux2/src/app/comum/citemimprimeestadourna.h (path inferred from the class name comum::CItemImprimeEstadoUrna)
// Reconstructed from vota_web_wasm.wasm by unit u37.
//
// "Mais informações" menu item: "Estado da urna (n/max)": prints the estado-da-urna report.
// RTTI: comum::CItemMenu <- comum::CItemImprimeEstadoUrna (vtable @1550788) <- vota::CItemImprimeEstadoUrnaVota (unit u02 / ctelasvota).
//   [2] Disponivel() const        wasm 11669 (this file)
//   [3] GetNumViasImpressas() = 0 implemented by vota::CItemImprimeEstadoUrnaVota (wasm 12267: EstadoGeralVota byte +73)
#pragma once

#include "comum/citemmenu.h"

namespace comum {

class CItemImprimeEstadoUrna : public CItemMenu {
public:
    using CItemMenu::CItemMenu;

    bool Disponivel() const override;                                // wasm 11669

protected:
    /// Copies already printed (vota.bin, EstadoGeralVota.numViasImpressasRelatorios).     name inferred
    virtual unsigned GetNumViasImpressas() const = 0;                // slot 3
};

} // namespace comum
