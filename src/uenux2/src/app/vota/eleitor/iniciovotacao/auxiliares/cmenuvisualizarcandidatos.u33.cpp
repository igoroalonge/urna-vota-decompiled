// uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.cpp  -- FRAGMENT written by
// unit u33 (srcloc cmenuvisualizarcandidatos.cpp:55 exists; the class is reconstructed by unit u09).
#include <memory>
#include <mutex>

#include "vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.h"

namespace vota {

// wasm func 2288                                                                    // name inferred
// Lazy singleton of the "Visualização de candidatos" menu (candidate viewer of the urna's pre-voting
// options): vota_f764(&mutex @1834608, &s_inst @1834632, vtable @1545600, CAppState flags 2), the merged
// body shared by the small vota states: `if (!s_inst) s_inst.reset(new C /*CAppState(flags), 12 bytes*/)`.
// Callers (all go back to this menu): CVisualizarCandidatos::StartState (11876), the three filter menus
// CMenuFiltrarCandidatosPorPartido / PorCargo (StartState) and PorNumero (slot 7), CMaisInformacoes (slot 7).
CMenuVisualizarCandidatos& CMenuVisualizarCandidatos::GetInst()
{
    static std::mutex mutex;                                    // @1834608 (unlock stub only)
    static std::unique_ptr<CMenuVisualizarCandidatos> s_inst;   // @1834632
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CMenuVisualizarCandidatos());          // comum::CAppState(2), vtable @1545600
    return *s_inst;
}

} // namespace vota
