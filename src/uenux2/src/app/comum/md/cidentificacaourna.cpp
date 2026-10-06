// uenux2/src/app/comum/md/cidentificacaourna.cpp
// Reconstructed from vota_web_wasm.wasm (unit u24).
#include "comum/md/cidentificacaourna.h"

#include "comum/md/cabrangencia.h"      // CUeComumMdError

namespace comum::md {

// wasm func 5886 (srclocs lines 25, 28, 31). Callers: CIdentificacaoSecao (5887, tipo '1') and the
// contingency-urna constructor 5888 (tipo '2'). All comparisons are unsigned.
CIdentificacaoUrna::CIdentificacaoUrna(const EUrnaTipo tipo, const TMunicipioID municipio, const TZonaID zona)
    : m_tipo(tipo), m_municipio(municipio), m_zona(zona)
{
    if (tipo < EUrnaTipo{'1'} || tipo > EUrnaTipo{'4'})              // compiled as (tipo - 53) >u -5
        throw CUeComumMdError(EUeComumMdError{8918}, "Tipo inválido.");                  // line 25
    if (municipio >= 100000)
        throw CUeComumMdError(EUeComumMdError{8919}, "Município inválido.");             // line 28
    if (zona >= 10000)
        throw CUeComumMdError(EUeComumMdError{8920}, "Zona inválida.");                  // line 31
}

} // namespace comum::md
