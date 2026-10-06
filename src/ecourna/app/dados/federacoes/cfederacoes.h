// ecourna-lib/ecourna/app/dados/federacoes/cfederacoes.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u11).
#pragma once

#include <vector>

#include "ecourna/app/dados/ccabecalhoentidade.h"
#include "ecourna/app/dados/federacoes/cfederacao.h"   // CFederacao (cfederacao.cpp, unit u14)

namespace ecourna::app::dados {

// Contents of a "-fe.dat" file (EntidadeFederacoes) after conversion. Non-polymorphic.
class CFederacoes
{
public:
    CFederacoes(const CCabecalhoEntidade& cabecalho, const std::vector<CFederacao>& federacoes);   // wasm func 9045

    const CCabecalhoEntidade& GetCabecalho() const { return m_cabecalho; }            // name inferred
    const std::vector<CFederacao>& GetFederacoes() const { return m_federacoes; }     // name inferred

private:
    CCabecalhoEntidade m_cabecalho;          // +0  (16 bytes)
    std::vector<CFederacao> m_federacoes;    // +16 (40-byte items)
};

}  // namespace ecourna::app::dados
