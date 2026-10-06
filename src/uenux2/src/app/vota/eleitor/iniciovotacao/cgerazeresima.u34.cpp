// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original files (paths inferred; see cgerazeresima.u07.cpp for the class CGeraZeresimaBase):
//   uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.cpp               (CGeraZeresima::GetInst)
//   uenux2/src/app/vota/eleitor/iniciovotacao/creimprimindoresumozeresima.cpp (attested file; GetInst only)
//
// WEB BUILD: never reached.
#include <memory>
#include <mutex>

#include "vota/eleitor/iniciovotacao/cgerazeresima.h"
#include "vota/eleitor/iniciovotacao/creimprimindoresumozeresima.h"

namespace vota {

// wasm func 5962 (tools: api_f5962). Lazy singleton: static unique_ptr @1834212, mutex @1834188.
// The object is built by CGeraZeresimaBase's constructor (func 5965, 20 bytes) and gets the vtable
// vota::CGeraZeresima @1544584; a previous instance is destroyed with CGeraZeresimaBase's dtor (func 1720).
// Callers: CConfirmaImpressaoZeresima::ProcessInput (11927), CImpressaoZeresimaTardia::ProcessInput (11931).
CGeraZeresima& CGeraZeresima::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CGeraZeresima> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CGeraZeresima());
    return *s_inst;
}

// wasm func 5942 (tools: api_f5942). One-line thunk into the merged lazy-singleton body vota_f764
// (mutex @1834776, instance @1834800, vtable CReimprimindoResumoZeresima @1546204, CAppState flags 0).
// Callers: CQuerReimprimirZeresima::ProcessInput (11848), CReimprimindoZeresima::ImprimeZeresima (11852).
CReimprimindoResumoZeresima& CReimprimindoResumoZeresima::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CReimprimindoResumoZeresima> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CReimprimindoResumoZeresima());        // CAppState(0), 12 bytes
    return *s_inst;
}

}  // namespace vota
