// FRAGMENT reconstructed by unit u09 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/eleitor/cdefinerotaposreinicio.cpp (class reconstructed by unit u06).
// The tools attributed this accessor to creimprimindoresumozeresima.cpp (its caller).

#include "vota/eleitor/cdefinerotaposreinicio.h"

#include <memory>
#include <mutex>

namespace vota {

// wasm func 5943 — lazy singleton (callers: CReinicioVotacao::ProcessInput, CReimprimindoResumoZeresima)
CDefineRotaPosReinicio& CDefineRotaPosReinicio::GetInst()
{
    static std::unique_ptr<CDefineRotaPosReinicio> s_inst;   // @1834772 (mutex residue @1834748)
    static std::mutex s_mutex;
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_inst)
        s_inst.reset(new CDefineRotaPosReinicio());           // 16 bytes: CAppState(7), +12 = 0
    return *s_inst;
}

}  // namespace vota
