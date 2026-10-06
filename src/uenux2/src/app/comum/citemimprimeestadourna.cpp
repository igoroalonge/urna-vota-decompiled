// uenux2/src/app/comum/citemimprimeestadourna.cpp (path inferred from the class name)
// Reconstructed from vota_web_wasm.wasm by unit u37.
#include "comum/citemimprimeestadourna.h"

#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/informacao/cinformacaoeleicao.h"

namespace comum {

// wasm func 11669 (vtable slot 2) - observed executing: CMenuBase::AdicionaItem (2285) and CMenuBase::Monta
// (5913) call it while CTelasVota builds the "Mais informações" form at start-up.
// Available while fewer copies were printed than the -pu.dat allows (ParametrosUrna.numRelatorioEstado,
// 1 in demonstration mode). The comparison is unsigned in the binary.
bool CItemImprimeEstadoUrna::Disponivel() const
{
    const CInformacaoEleicao informacao(CConfiguracaoEleicao::GetInst());          // vota_f603 (cfg +88)
    const unsigned maximo = informacao.GetNumRelatorioEstado();                     // wasm 2865
    return GetNumViasImpressas() < maximo;                                           // slot 3 (+73)
}

} // namespace comum
