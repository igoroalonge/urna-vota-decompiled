// FRAGMENT reconstructed by unit u19 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/iexecucaovota.cpp (srcloc :74, IExecucaoVota::GetInst()).
//
// vota::IExecucaoVota = "how the voting application runs its threads". The urna default is
// vota::CExecucaoVota (real threads, vtable @1600580, 4 bytes); the web build registers
// vota::CExecucaoVotaCooperativa from main() before anything asks for it, so the default below is never
// created in the simulator (the trace in docs/modules/u19 shows IExecucaoVota pushed by main at sz[12]).
#include "vota/iexecucaovota.h"

#include <memory>
#include <mutex>
#include <source_location>

#include "api/pattern/cpolysingleton.h"
#include "vota/cexecucaovota.h"

namespace vota {

// wasm func 3594 (tools name "api::CPolySingletonList::instance@3594"; table slot 54).
// Called by votaInit, votaTick, CThreadEleitor::vf5 and two wasm_entry helpers.
IExecucaoVota& IExecucaoVota::GetInst()
{
    static std::mutex s_mutex;                                              // @1911520 (unlock residue)
    std::lock_guard trava(s_mutex);
    if (!api::CPolySingleton<IExecucaoVota>::exists())                       // func 2736
        api::CPolySingletonList::push<IExecucaoVota>(std::make_unique<CExecucaoVota>(),
                                                     api::GetPolySingletonsInfo());   // push = func 5395 (no
                                                                                      // exists/erase prefix)
    return api::CPolySingleton<IExecucaoVota>::instance(api::GetPolySingletonsInfo(),
                                                        std::source_location::current());   // :74
}

}  // namespace vota
