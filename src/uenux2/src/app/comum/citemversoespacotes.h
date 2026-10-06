// uenux2/src/app/comum/citemversoespacotes.h (path inferred from the class name comum::CItemVersoesPacotes)
// Reconstructed from vota_web_wasm.wasm by unit u37.
//
// "Mais informações" menu item: "Versões de pacotes (n/max)": prints the versions of the data packages.
// RTTI: comum::CItemMenu <- comum::CItemVersoesPacotes (vtable @1550896) <- vota::CItemVersoesPacotesVota (unit u02 / ctelasvota).
//   [2] Disponivel() const        wasm 11666 (this file)
//   [3] GetNumViasImpressas() = 0 implemented by vota::CItemVersoesPacotesVota (wasm 12259: EstadoGeralVota byte +75)
#pragma once

#include "comum/citemmenu.h"

namespace comum {

class CItemVersoesPacotes : public CItemMenu {
public:
    using CItemMenu::CItemMenu;

    bool Disponivel() const override;                                // wasm 11666

protected:
    /// Copies already printed (vota.bin, EstadoGeralVota.numViasImpressasRelatorios).     name inferred
    virtual unsigned GetNumViasImpressas() const = 0;                // slot 3
};

} // namespace comum
