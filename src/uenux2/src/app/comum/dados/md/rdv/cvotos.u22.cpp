// uenux2/src/app/comum/dados/md/rdv/cvotos.cpp  --  FRAGMENT written by unit u22
// Only CVotos::Insere (srcloc cvotos.cpp:40) is here: it has no wasm function of its own, it is
// inlined into vota::CEleitorVotando::GravaVotos (func 4454). The constructor (func 2794) is described
// in u05. Merge into cvotos.cpp.
#include "comum/dados/md/rdv/cvotoscargos.h"

#include <string>

#include "comum/dados/rdvdefs.h"   // CRdvError

namespace comum::md {

// srcloc cvotos.cpp:40 - inserts `voto` at `posicao` (0 .. size()).
void CVotos::Insere(const CVoto& voto, std::size_t posicao)
{
    if (posicao > m_votos.size())
        throw CRdvError(api::EUeRdvError{4662},
            "Posição (" + std::to_string(posicao) + ") fora do limite (" +
            std::to_string(static_cast<std::uint16_t>(m_votos.size())) + ")");     // :40 (size printed as uint16)
    m_votos.insert(m_votos.begin() + static_cast<std::ptrdiff_t>(posicao), voto);   // vector insert, inlined
}

} // namespace comum::md
