// uenux2/src/app/comum/md/cidentificacaournacontingencia.h  (path inferred from the .cpp, attested by the
// std::source_location record @1552708 "cidentificacaournacontingencia.cpp:32")
// Reconstructed from vota_web_wasm.wasm (unit u36).
//
// md::CIdentificacaoUrnaContingencia = identification of a contingency urna ("urna de contingência": a spare
// urna that replaces a broken one and is not tied to one section). It carries only município and zona,
// mirroring ModuloTiposEleitorais::IdentificacaoUrna.identificacaoContingencia. sizeof 12 (same as the base).
#pragma once

#include "comum/md/cidentificacaourna.h"

namespace comum::md {

class CIdentificacaoUrnaContingencia : public CIdentificacaoUrna {
public:
    // wasm func 5888. Every caller passes tipo = '2', so the optimiser removed the parameter.
    CIdentificacaoUrnaContingencia(const EUrnaTipo tipo, const TMunicipioID municipio, const TZonaID zona);
};

}  // namespace comum::md
