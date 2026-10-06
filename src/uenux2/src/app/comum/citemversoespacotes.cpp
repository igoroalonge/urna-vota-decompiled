// uenux2/src/app/comum/citemversoespacotes.cpp (path inferred from the class name)
// Reconstructed from vota_web_wasm.wasm by unit u37.
#include "comum/citemversoespacotes.h"

#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/informacao/cinformacaoeleicao.h"

namespace comum {

// wasm func 11666 (vtable slot 2). The report itself is comum::CGeradorRelVersaoPacoteDados (unit u02/u25).
bool CItemVersoesPacotes::Disponivel() const
{
    const CInformacaoEleicao informacao(CConfiguracaoEleicao::GetInst());
    const unsigned maximo = informacao.GetNumRelatorioVersoesDados();               // wasm 5916
    return GetNumViasImpressas() < maximo;                                           // slot 3 (+75)
}

} // namespace comum
