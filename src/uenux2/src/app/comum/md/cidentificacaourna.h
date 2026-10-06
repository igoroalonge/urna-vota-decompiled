// uenux2/src/app/comum/md/cidentificacaourna.h
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// md::CIdentificacaoUrna = which urna produced a result: its type and the município / zona it serves.
// Base of md::CIdentificacaoSecao (seção urna, tipo '1') and md::CIdentificacaoUrnaContingencia
// (cidentificacaournacontingencia.cpp:32, tipo '2', constructor = wasm 5888, other unit). Mirrors
// ModuloTiposEleitorais::IdentificacaoUrna CHOICE { identificacaoSecaoEleitoral, identificacaoContingencia }.
// Not polymorphic. sizeof 12 (10 used).
#pragma once

#include "comum/comumtypes.h"          // EUrnaTipo, TMunicipioID, TZonaID   (header name ?)

namespace comum::md {

class CIdentificacaoUrna {
public:
    CIdentificacaoUrna(const EUrnaTipo tipo, const TMunicipioID municipio, const TZonaID zona);   // wasm 5886

    EUrnaTipo GetTipo() const { return m_tipo; }
    TMunicipioID GetMunicipio() const { return m_municipio; }
    TZonaID GetZona() const { return m_zona; }

protected:
    EUrnaTipo    m_tipo;         // +0  char code '1'..'4' stored as int ('1' seção, '2' contingência)
    TMunicipioID m_municipio;    // +4  < 100000
    TZonaID      m_zona;         // +8  uint16, < 10000
};

} // namespace comum::md
