// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/eleitor/iniciovotacao/cconfirmaregerarzeresima.cpp  (path inferred)
// Class declaration: estadosiniciovotacao.u34.h. Also here: CRegerarZeresima's lazy singleton (inlined into
// 11838, original file probably cregerarzeresima.cpp or cgerazeresima.cpp).
//
// WEB BUILD: never reached.
#include <memory>
#include <mutex>

#include "vota/eleitor/iniciovotacao/cgerazeresima.h"             // CGeraZeresimaBase, CRegerarZeresima (u07)
#include "vota/eleitor/iniciovotacao/estadosiniciovotacao.u34.h"

namespace vota {

// GetInst - inlined into wasm func 11838: static unique_ptr @1834912, mutex @1834888. The object is a
// CGeraZeresimaBase (20 bytes, ctor func 5965: CAppState + the "gerando zerésima" screen, CTelasVota +212)
// whose vtable is then set to CRegerarZeresima @1546500 (slot 9 = 11842: cut the paper after printing).
// The old instance, if any, is destroyed through CGeraZeresimaBase's destructor (func 1720).
CRegerarZeresima& CRegerarZeresima::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CRegerarZeresima> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CRegerarZeresima());
    return *s_inst;
}

// wasm func 11838 - vtable slot 7 (ProcessInput). srcloc cinteractiveform.h:57 @1546612.
void CConfirmaRegerarZeresima::ProcessInput()
{
    switch (m_tela->Read()) {
    case api::EInputResult::CONFIRMA:                           // generate and print the zerésima again
        m_proximoEstado = &CRegerarZeresima::GetInst();
        break;
    case api::EInputResult::BRANCO:                             // "mais informações"
        m_proximoEstado = &CMaisInformacoes::GetInst(this);     // func 1280
        break;
    default:
        break;
    }
}

}  // namespace vota
