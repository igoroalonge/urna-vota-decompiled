// uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp (attested by srcloc :40) -- FRAGMENT written by
// unit u37. The class and its other getters are in cinformacaoeleicao.cpp (unit u23).
#include "comum/informacao/cinformacaoeleicao.h"

namespace comum {

// wasm func 5918 (tools: vota_f5918)                     name as in units u09/u23/u27 (name inferred)
// ParametrosUrna.aceitarJustificativa (CParametrosUrna +394 = CConfiguracaoEleicao +482).
// Callers: vota::CImprimirBUOutrasObrigatorias::StartState (12065: print the "boletim de justificativa"
// after the mandatory BU copies) and vota::(anon)::JustificativaNaoAceita (4578, operator terminal).
// NOTE: unlike every other getter of this class it has NO demonstration-mode override.
bool CInformacaoEleicao::ImprimeBoletimJustificativa() const
{
    return m_parametros->m_aceitarJustificativa;
}

} // namespace comum
