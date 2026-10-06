// FRAGMENT reconstructed by unit u07 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/eleitor/iniciovotacao/cquerreimprimirzeresima.cpp (attested, srcloc :45 StartState).
// vota::CQuerReimprimirZeresima (vtable @1546348) - "do you want to reprint the zerésima?". Merge into the .cpp.
#include "vota/eleitor/iniciovotacao/cquerreimprimirzeresima.h"

#include <mutex>

#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

static std::unique_ptr<CQuerReimprimirZeresima> s_pInstancia;   // @1834856
static std::mutex s_mutexInstancia;                               // @1834832

// wasm func 5940 (called by CReinicioVotacao::StartState)                            // name inferred
CQuerReimprimirZeresima& CQuerReimprimirZeresima::GetInst()
{
    std::lock_guard<std::mutex> lock(s_mutexInstancia);
    if (!s_pInstancia)
        s_pInstancia.reset(new CQuerReimprimirZeresima());    // CAppState(2), m_tela = CTelasVota::GetInst().m_telaQuerReimprimirZeresima (+244)
    return *s_pInstancia;
}

} // namespace vota
