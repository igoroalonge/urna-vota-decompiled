// ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrizacaourna.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u11).
#pragma once

#include "ecourna/app/dados/ccabecalhoentidade.h"
#include "ecourna/app/dados/parametrizacaourna/cparametrosurna.h"

namespace ecourna::app::dados {

// Contents of a "-pu.dat" file (EntidadeParametrizacaoUrna) after conversion.
class CParametrizacaoUrna
{
public:
    CParametrizacaoUrna(const CCabecalhoEntidade& cabecalho, const CParametrosUrna& parametros);   // wasm func 9029

    const CCabecalhoEntidade& GetCabecalho() const { return m_cabecalho; }     // name inferred (inlined)
    const CParametrosUrna& GetParametros() const { return m_parametros; }      // name inferred (inlined)

private:
    CCabecalhoEntidade m_cabecalho;    // +0  (16 bytes)
    CParametrosUrna m_parametros;      // +16
};

}  // namespace ecourna::app::dados
