// uenux2/src/app/comum/md/cidentificacaosecao.h
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// md::CIdentificacaoSecao = identification of a "seção eleitoral" (polling station) urna:
// município / zona (base CIdentificacaoUrna, tipo '1') + local de votação + seção.
// = ModuloTiposEleitorais::IdentificacaoSecaoEleitoral. Not polymorphic. sizeof 20 (18 used).
#pragma once

#include "comum/md/cidentificacaourna.h"

namespace comum::md {

class CIdentificacaoSecao : public CIdentificacaoUrna {
public:
    CIdentificacaoSecao(const EUrnaTipo tipo, const TMunicipioID municipio, const TZonaID zona,
                        const TLocalID local, const TSecaoID secao);                    // wasm 5887

    TLocalID GetLocal() const { return m_local; }
    TSecaoID GetSecao() const { return m_secao; }

private:
    TLocalID m_local;    // +12 < 10000 (0 allowed)
    TSecaoID m_secao;    // +16 uint16, 1..9999
};

} // namespace comum::md
