// FRAGMENT of uenux2/src/app/comum/dados/ccandidaturas.cpp (attested; owner u03). Reconstructed from vota_web_wasm.wasm
// (unit u35).
//
// The CCandidaturasDS* functors are the "data sources" (DS) of the voting screens and reports: small objects that read
// the CURRENT candidacy of CCandidaturas when the text/image is drawn. CCandidaturasDSSexo has no RTTI (it is never
// stored in a std::function); its sibling CCandidaturasDSNumero has.
#include "comum/dados/ccandidaturas.h"

namespace comum {

// wasm func 5806 - name from the context string it passes to GetCandidaturaAtual. Observed executing (every
// confirmation screen: the office title agrees with the candidate's sex, "Prefeito" / "Prefeita").
// Callers: CCargoDSNomeSexoCandidato::operator() (2268), vota::(anonymous)::DS_CandidatoNaoConcorre (13101).
// m_suplente (+0, uebyte): 0 = the titular, n = the running mate of ordem n (vice / suplente).
md::CSexo::ESexo CCandidaturasDSSexo::operator()() const
{
    const md::CCandidatura& candidatura = GetCandidaturaAtual("CCandidaturasDSSexo");      // func 2838 (:261)
    const md::CDadosCandidato& dados = (m_suplente != 0) ? candidatura.GetSuplente(m_suplente)   // func 1389 (:65)
                                                         : candidatura.GetTitular();            // +8
    return dados.GetSexo();                                                                // CDadosCandidato +40
}

} // namespace comum
