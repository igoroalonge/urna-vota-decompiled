// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/comum/dados/ccandidaturas.cpp (attested by srcloc; owner unit u03).
//
// comum::CCandidaturasDSNome: data source ("DS") of the candidate's name on the voter's confirmation screen,
// used as CDataText<comum::CCandidaturasDSNome> (GetText = func 12649, observed executing) and by the
// "Vice-prefeito: <name>" / "1º suplente: <name>" lambdas of ccargods.cpp (merged body 6076).
// Layout: 1 byte, +0 uebyte m_indice (0 = the candidate; 1, 2 = vice / suplentes).
#include <string>

#include "comum/dados/ccandidaturas.h"

namespace comum {

// wasm func 5807 (tools: api_f5807)                                                name inferred (operator())
std::string CCandidaturasDSNome::operator()() const
{
    // GetCandidaturaAtual (func 2838, ccandidaturas.cpp:261) throws CDadosError 7805
    // "CCandidaturasDSNome - não posicionado no candidato corretamente" when no candidacy is current.
    const md::CCandidatura& candidatura = GetCandidaturaAtual("CCandidaturasDSNome");   // @182017
    const md::CCandidato& candidato = m_indice != 0 ? candidatura.GetSuplente(m_indice)  // func 1389
                                                    : candidatura.GetTitular();          // +8
    return candidato.GetNomeUrna();                                                       // std::string at +12
}

}  // namespace comum
