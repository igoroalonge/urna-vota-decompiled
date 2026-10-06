// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/eleitor/celeitordecorator.h
#pragma once

#include <compare>

#include "celeitor.h"

namespace comum::md {

class CEleitorDecorator {
public:
    // inlined in 5772: the CEleitor is copy-constructed straight into the decorator (func 946), so
    // the parameter is a const reference (a by-value parameter would add a copy + a move)
    CEleitorDecorator(const CEleitor& eleitor, ETipoIdentificadorEleitor tipoPrincipal)
        : m_eleitor(eleitor), m_tipoPrincipal(tipoPrincipal) {}

    CEleitorIdentidade GetIdentidadePorTipo(ETipoIdentificadorEleitor tipo) const;   // func 708
    CEleitorIdentidade GetIdentidadePrincipal() const { return GetIdentidadePorTipo(m_tipoPrincipal); }
    std::strong_ordering operator<=>(const CEleitorDecorator& outro) const;         // func 456

private:
    CEleitor m_eleitor;                          // +0
    ETipoIdentificadorEleitor m_tipoPrincipal;   // +104 (int-sized)
};

}  // namespace comum::md
