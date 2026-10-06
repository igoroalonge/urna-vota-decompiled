// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/correspondencia/ccorrespondenciaresultado.h
#pragma once

#include <cstdint>

#include "ccarga.h"

namespace comum::md {

using TMunicipioID = std::uint32_t;
using TZonaID = std::uint16_t;
using TSecaoID = std::uint16_t;

// ? values are ASCII digits; the ASN.1 TipoUrna (secao=1, contingencia=3, ...) is mapped by the
// converter (CConversorCorrespResultado returns '1' for identificacao choice 0, '2' for choice 1).
enum class ETipoUrna : int { SECAO = '1', CONTINGENCIA = '2', TIPO3 = '3', TIPO4 = '4' };

class CCorrespondenciaResultado {
public:
    CCorrespondenciaResultado(TMunicipioID municipio, TZonaID zona, TSecaoID secao,
                              const CCarga& carga, ETipoUrna tipoUrna);   // func 2801
    void ValidaCriacao() const;                                           // inlined into 2801

private:
    TMunicipioID m_municipio;   // +0
    TZonaID m_zona;             // +4
    TSecaoID m_secao;           // +6
    CCarga m_carga;             // +8
    ETipoUrna m_tipoUrna;       // +84
};

}  // namespace comum::md
