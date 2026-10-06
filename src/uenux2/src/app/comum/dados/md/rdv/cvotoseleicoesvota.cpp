// uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by std::source_location records
// :33 / :40 (MontaMapa and its nested lambda), :118 (Comparecimento), :215 (RecebeCedula, inlined into
// func 4454). The other counters (:113 GetVotos, :126 Candidato, :140 Legenda, :154 Partido, :166
// Nominais, :172 Legendas, :178 Nulos, :184 Brancos, :190 Cargo) are listed in other units (u05).
//
// comum::md::CVotosEleicoesVota: +0 TMapa m_mapa (map<TEleicaoID, CVotosCargos>)
//                                +12 TMapaCargoEleicao m_cargoEleicao (map<TCargoID, TEleicaoID>)
#include "comum/dados/md/rdv/cvotoseleicoesvota.h"

#include <algorithm>
#include <source_location>
#include <string>

#include "comum/dados/rdvdefs.h"   // CRdvError

namespace comum::md {

namespace {

// wasm func 3708 - map lookup shared by the non-const accessors; the caller passes its own error code
// and std::source_location (4734 + :118 Comparecimento, 4735 + :215 RecebeCedula, and
// CRdvVota::vf1 func 11490 with :113). Same shape as func 3707 for cargos (crdvvota.cpp).   name inferred
template <class MAPA>
auto& BuscaEleicao(MAPA& mapa, api::EUeRdvError erro, TEleicaoID eleicao,
                   const std::source_location& local)
{
    const auto it = mapa.find(eleicao);
    if (it == mapa.end())
        throw CRdvError(erro, "Eleicao (" + std::to_string(eleicao) + ") nao encontrada", local);   // func 327 = to_string(unsigned)
    return it->second;
}

// srcloc :33 / :40 - which eleição each cargo belongs to; a cargo may appear in only one eleição.
CVotosEleicoesVota::TMapaCargoEleicao MontaMapa(const CVotosEleicoesVota::TMapa& mapa)
{
    if (mapa.empty())
        throw CRdvError(api::EUeRdvError{4723}, "Grupo de votos vazio");                              // :33
    CVotosEleicoesVota::TMapaCargoEleicao resultado;
    std::ranges::for_each(mapa, [&](const std::pair<const unsigned, CVotosCargos>& eleicao) {
        std::ranges::for_each(eleicao.second.GetMapa(), [&](const auto& it) {
            if (!resultado.emplace(it.first.codigo, eleicao.first).second)
                throw CRdvError(api::EUeRdvError{4724},
                    "Cargo " + std::to_string(it.first.codigo) + " presente em mais de uma eleicao");   // :40
        });
    });
    return resultado;
}

} // namespace

// wasm func 5640 (tools: "MontaMapa", whose body is inlined here) - observed executing
// (CConversorEleicoesVota::DoDesconverte when the RDV is loaded, and the start-up routine 7787).
// name inferred
CVotosEleicoesVota::CVotosEleicoesVota(TMapa mapa)
    : m_mapa(std::move(mapa))
    , m_cargoEleicao(MontaMapa(m_mapa))
{
}

// wasm func 5639 (srcloc :118): number of ballots cast in the eleição (comparecimento).
TQtdVoto CVotosEleicoesVota::Comparecimento(TEleicaoID eleicao) const
{
    return BuscaEleicao(m_mapa, api::EUeRdvError{4734}, eleicao, std::source_location::current())   // :118
        .Comparecimento();                                                      // first cargo: votes / qtdEscolhas
}

// srcloc :215 - inlined into func 4454 (vota::CEleitorVotando::GravaVotos).
void CVotosEleicoesVota::RecebeCedula(const CRdvPosicionador& posicionador, TEleicaoID eleicao,
                                      const CCedula& cedula)
{
    CVotosCargos& votos = BuscaEleicao(m_mapa, api::EUeRdvError{4735}, eleicao,
                                       std::source_location::current());       // :215
    votos.ConfereCedula(cedula);
    votos.InsereCedula(posicionador, cedula);
}

} // namespace comum::md
