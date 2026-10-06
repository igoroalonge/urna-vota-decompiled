// FRAGMENT of uenux2/src/app/comum/dados/clocal.cpp (attested; file of unit u04, declarations in clocal.h).
// Reconstructed from vota_web_wasm.wasm (unit u35). Three accessors of the singleton comum::CLocal (where this urna is
// installed). Like every accessor of the class they first call VerificaLido(<own name>) (clocal.cpp:345: throws when
// the -lo.dat file was not loaded yet) or VerificaEhSecao(<own name>) (clocal.cpp:354: also throws for a
// contingency urna, which has no section).
#include "comum/dados/clocal.h"

#include <vector>

namespace comum {

// wasm func 5741 - name from the string passed to VerificaLido. Observed executing (start-up loader 7787, which
// builds the HKDF seed and the per-section file names; CGeraRelatorios 12105).
// Returns the principal section followed by the aggregated sections ("seções agregadas"), or an empty list for a
// contingency urna. The principal entry carries the principal section's TipoLocalVotacao.
std::vector<md::CIdentificacaoAgregada> CLocal::GetTodasSecoes() const
{
    VerificaLido("GetTodasSecoes");                                              // line 345 (func 782)
    std::vector<md::CIdentificacaoAgregada> secoes;
    if (m_local->EhSecao()) {                                                    // optional<CSecaoEleitoral> flag +120
        const md::CSecaoEleitoral& secao = m_local->GetSecao();                  // func 943
        const std::vector<md::CIdentificacaoAgregada> agregadas = secao.GetAgregadas();   // copied twice in the binary
        secoes.push_back(md::CIdentificacaoAgregada(secao.GetSecao(), secao.GetTipo()));  // func 5672
        secoes.insert(secoes.end(), agregadas.begin(), agregadas.end());
    }
    return secoes;
}

// wasm func 1933 - name from the string passed to VerificaEhSecao. Observed executing (start-up loader 7787, which
// uses it for the HKDF seed); also used at the encerramento (CGravaResultado, BU QR code "LOCA:", report headers).
// The "local de votação" number (polling place) of the section: CSecaoEleitoral +16.
TLocalID CLocal::GetLocalID() const
{
    VerificaEhSecao("GetLocalID");                                               // line 354 (func 5742)
    return m_local->GetSecao().GetLocal();
}

// wasm func 2816 - name from the string passed to VerificaLido. Not observed executing.
// Number of aggregated sections (0 for a contingency urna). Returned as uebyte (size & 0xFF).
uebyte CLocal::GetQtdAgregadas() const
{
    VerificaLido("GetQtdAgregadas");                                             // line 345
    if (!m_local->EhSecao())
        return 0;
    return static_cast<uebyte>(m_local->GetSecao().GetAgregadas().size());
}

} // namespace comum
