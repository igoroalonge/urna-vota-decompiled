// uenux2/src/app/comum/citemparametrosurna.cpp (path inferred from the class name)
// Reconstructed from vota_web_wasm.wasm by unit u37.
#include "comum/citemparametrosurna.h"

#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/informacao/cinformacaoeleicao.h"

namespace comum {

// wasm func 11667 (vtable slot 2). Selecting the item leads to vota::CImpressaoPU (the "Parâmetros de urna"
// report, wasm 11902, which prints the time windows with CGeradorRelPU::AdicionaLinha, wasm 677).
bool CItemParametrosUrna::Disponivel() const
{
    const CInformacaoEleicao informacao(CConfiguracaoEleicao::GetInst());
    const unsigned maximo = informacao.GetNumRelatorioPU();                         // wasm 5915
    return GetNumViasImpressas() < maximo;                                           // slot 3 (+76)
}

} // namespace comum
