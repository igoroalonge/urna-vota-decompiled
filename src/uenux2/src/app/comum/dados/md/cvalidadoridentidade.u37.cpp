// uenux2/src/app/comum/dados/md/cvalidadoridentidade.cpp (attested by srclocs :26, :61) -- FRAGMENT written
// by unit u37. Class: cvalidadoridentidade.h (unit u05).
#include <algorithm>
#include <string>

#include "comum/dados/md/cvalidadoridentidade.h"

namespace comum::md {

// wasm func 2803 (tools: vota_f2803)                                                   name inferred
// Non-throwing twin of Valida(): same rule search (the first rule that is the "free identifier" rule,
// tipo 3, or has the requested type), but answers false instead of throwing 8108 when no rule matches.
// Callers: comum::(anon)::GetControlador (10313, mesário registration), vota::CPerguntaCodigoSuspensao
// slot 7 (10420: "Título {} é válido/inválido para suspender a votação") and
// vota::CPedeTituloEncerramento slot 7 (10721) and vota_f2225; the call sites checked pass tipo 1 (título).
bool CValidadorIdentidade::EhValida(ETipoIdentificadorEleitor tipo, const std::string& identidade) const
{
    const auto regra = std::find_if(m_regras.begin(), m_regras.end(), [tipo](const auto& r) {
        return r->GetTipo() == ETipoIdentificadorEleitor::LIVRE /*3*/ || r->GetTipo() == tipo;   // slot 2
    });
    if (regra == m_regras.end())
        return false;
    return (*regra)->EhValida(identidade);                                          // wasm 3723
}

} // namespace comum::md
