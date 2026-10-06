// FRAGMENT reconstructed by unit u07 from vota_web_wasm.wasm.
// Original file (path inferred): uenux2/src/app/vota/eleitor/iniciovotacao/cquerimprimirzeresima.cpp
// vota::CQuerImprimirZeresima (vtable @1544848; other methods in unit u39/u20) - "do you want to print the zerésima?".
#include "vota/eleitor/iniciovotacao/cquerimprimirzeresima.h"

#include <mutex>

#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

static std::unique_ptr<CQuerImprimirZeresima> s_pInstancia;   // @1834324
static std::mutex s_mutexInstancia;                             // @1834300

// wasm func 5957 (called by CReinicioVotacao::StartState and CInicioZeresima::StartState, func 11924)  // name inferred
CQuerImprimirZeresima& CQuerImprimirZeresima::GetInst()
{
    std::lock_guard<std::mutex> lock(s_mutexInstancia);
    if (!s_pInstancia)
        s_pInstancia.reset(new CQuerImprimirZeresima());      // CAppState(2), m_tela = CTelasVota::GetInst().m_telaQuerImprimirZeresima (+228)
    return *s_pInstancia;
}

} // namespace vota
