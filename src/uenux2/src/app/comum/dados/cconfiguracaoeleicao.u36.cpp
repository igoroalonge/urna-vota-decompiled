// uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp  --  FRAGMENT written by unit u36 (file owned by u04).
// Reconstructed from vota_web_wasm.wasm.
#include "comum/dados/cconfiguracaoeleicao.h"

#include <vector>

#include "comum/dados/md/processoeleitoral/ccargo.h"
#include "comum/dados/md/processoeleitoral/celeicaope.h"

namespace comum {

// 8 bytes, the same record CCargos keeps (CCargos::SCargoEleicao): the eleição a cargo belongs to.
struct SEleicaoCargo {                  // name inferred
    TEleicaoID eleicao;                 // +0  CEleicaoPE +0
    TCargoID   cargo;                   // +4  CCargo +0 (código)
};

// wasm func 3774 - observed executing (votaInit and every confirmed vote)           // name inferred (u22)
// One (eleição, cargo) pair for every cargo of every eleição of the pleito, in file order:
// municipal elections give {(E, Prefeito), (E, Vereador)}, general elections two eleições (federal and
// estadual) with their own cargos. Callers:
//   * CCargos::CreateInst (inlined in the start-up function 7787): twice, m_todos and m_cargos;
//   * vota::CEleitorVotando::GravaVotos (4454): to find the eleição of each confirmed vote, i.e. which
//     cédula (ballot) of the RDV receives it (see celeitorvotando.u22.cpp).
// The pleito's vector<CEleicaoPE> (52-byte elements) sits at CConfiguracaoEleicao +52 (m_pleito +24);
// CEleicaoPE::GetCargos() is wasm 2257 (vector<CCargo>, 140-byte elements).
std::vector<SEleicaoCargo> CConfiguracaoEleicao::GetEleicoesCargos() const
{
    std::vector<SEleicaoCargo> pares;
    for (const md::CEleicaoPE& eleicao : m_pleito.GetEleicoes())
        for (const md::CCargo& cargo : eleicao.GetCargos())
            pares.push_back({eleicao.GetId(), cargo.GetCodigo()});
    return pares;
}

}  // namespace comum
