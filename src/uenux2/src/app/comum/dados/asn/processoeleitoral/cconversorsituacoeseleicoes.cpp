// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsituacoeseleicoes.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35).
#include "comum/dados/asn/processoeleitoral/cconversorsituacoeseleicoes.h"

namespace comum::asn {

// wasm func 11361 - vtable slot 3. Not in the observed list, although the scenarios ship a "-ste.dat" (e.g.
// t02410ac-ste.dat) that the start-up loader reads through this converter: a short function the sampler missed.
// Only ACTIVE elections ("ativa", value 0) are kept: a suspended / cancelled / excluded election simply disappears
// from the list (no log, no error). The cabeçalho is not looked at.
std::vector<md::CSituacoesEleicoes> CConversorSituacoesEleicoes::DoDesconverte(
    const ModuloSituacoesEleicoes::EntidadeSituacoesEleicoes& entidade) const
{
    std::vector<md::CSituacoesEleicoes> situacoes;
    for (const auto& situacao : entidade.get_situacoesEleicoes()) {      // field 1 (read even when absent: empty)
        if (situacao.get_tipo() != ModuloSituacoesEleicoes::TipoSituacaoEleicao::ativa)
            continue;
        situacoes.push_back(md::CSituacoesEleicoes{
            static_cast<TEleicaoID>(situacao.get_idEleicao()),
            static_cast<uebyte>(situacao.get_ordemAquisicao()),
            static_cast<uebyte>(situacao.get_ordemImpressao()),
            static_cast<uebyte>(situacao.get_ordemApuracao())});        // rhvoice_f760 = vector<8-byte>::push_back
    }
    return situacoes;
}

} // namespace comum::asn
