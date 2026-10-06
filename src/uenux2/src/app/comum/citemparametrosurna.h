// uenux2/src/app/comum/citemparametrosurna.h (path inferred from the class name comum::CItemParametrosUrna)
// Reconstructed from vota_web_wasm.wasm by unit u37.
//
// "Mais informações" menu item: "Parâmetros de urna (n/max)": prints the PU (parâmetros de urna) report.
// RTTI: comum::CItemMenu <- comum::CItemParametrosUrna (vtable @1550860) <- vota::CItemParametrosUrnaVota (unit u02 / ctelasvota).
//   [2] Disponivel() const        wasm 11667 (this file)
//   [3] GetNumViasImpressas() = 0 implemented by vota::CItemParametrosUrnaVota (wasm 12255: EstadoGeralVota byte +76)
#pragma once

#include "comum/citemmenu.h"

namespace comum {

class CItemParametrosUrna : public CItemMenu {
public:
    using CItemMenu::CItemMenu;

    bool Disponivel() const override;                                // wasm 11667

protected:
    /// Copies already printed (vota.bin, EstadoGeralVota.numViasImpressasRelatorios).     name inferred
    virtual unsigned GetNumViasImpressas() const = 0;                // slot 3
};

} // namespace comum
