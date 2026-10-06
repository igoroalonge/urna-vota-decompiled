// FRAGMENT reconstructed by unit u19 from vota_web_wasm.wasm.
// Original file: unknown - path inferred: uenux2/src/api/util/iajustedatahora.h (sibling of
// isystemdatetime.h / itimerscheduler.h; the implementation api::CAjusteDataHora is in
// src/uenux2/src/api/util/cajustedatahora.cpp, unit u05).
//
// api::IAjusteDataHora sets the urna clock (through ISystemDateTime). main() registers the default
// implementation through this function (table slot 118, `invoke_v` in func 10307).
#include "api/util/iajustedatahora.h"

#include <memory>

#include "api/pattern/cpolysingleton.h"
#include "api/util/cajustedatahora.h"

namespace api {

// wasm func 10876 (tools name "api::CPolySingletonList::push@10876"; table slot 118).     // name inferred
// Unlike ITimerScheduler/ISystemDateTime::CreateInst there is no "Tentativa de recriar o singleton"
// check: an existing registration is simply kept. The register-or-replace wrapper (by-value helper + replace +
// push, srcloc :129) is inlined: it calls exists<IAjusteDataHora> (2469) a second time and erases if true.
void IAjusteDataHora::CreateInst()
{
    if (!CPolySingleton<IAjusteDataHora>::exists())                        // func 2469
        CPolySingletonList::replace<IAjusteDataHora>(std::make_unique<CAjusteDataHora>(),   // vtable @1584164
                                                     GetPolySingletonsInfo());
}

}  // namespace api
