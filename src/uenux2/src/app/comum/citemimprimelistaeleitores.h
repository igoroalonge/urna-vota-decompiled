// uenux2/src/app/comum/citemimprimelistaeleitores.h (path inferred from the class name comum::CItemImprimeListaEleitores)
// Reconstructed from vota_web_wasm.wasm by unit u37.
//
// "Mais informações" menu item: "Lista de eleitores (n/max)": prints the list of the section's voters.
// RTTI: comum::CItemMenu <- comum::CItemImprimeListaEleitores (vtable @1550824) <- vota::CItemImprimeListaEleitoresVota (unit u02 / ctelasvota).
//   [2] Disponivel() const        wasm 11668 (this file)
//   [3] GetNumViasImpressas() = 0 implemented by vota::CItemImprimeListaEleitoresVota (wasm 12266: EstadoGeralVota byte +74)
#pragma once

#include "comum/citemmenu.h"

namespace comum {

class CItemImprimeListaEleitores : public CItemMenu {
public:
    using CItemMenu::CItemMenu;

    bool Disponivel() const override;                                // wasm 11668

protected:
    /// Copies already printed (vota.bin, EstadoGeralVota.numViasImpressasRelatorios).     name inferred
    virtual unsigned GetNumViasImpressas() const = 0;                // slot 3
};

} // namespace comum
