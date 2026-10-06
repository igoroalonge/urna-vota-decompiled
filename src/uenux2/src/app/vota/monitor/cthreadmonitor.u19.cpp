// FRAGMENT reconstructed by unit u19 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/monitor/cthreadmonitor.cpp (srcloc :103, the lambda of
// CThreadMonitor::SaiPorVotacaoSuspensa()).
#include <future>
#include <source_location>

#include "api/hwil/ibeep.h"
#include "api/pattern/cpolysingleton.h"

namespace vota {

// The lambda is run with std::async; its body is the __async_assoc_state<void, __async_func<$_0>>::__execute
// override (vtable slot 3), wasm func 10224: run the lambda, then set_value() (unknown_f4771).
// The monitor thread is not started in the web build.
void CThreadMonitor::SaiPorVotacaoSuspensa()
{
    // ...
    auto aviso = std::async(std::launch::async, [] {
        api::CPolySingleton<api::IBeep>::instance(api::GetPolySingletonsInfo(),
                                                  std::source_location::current())   // :103 (func 925)
            .vf4();                                        // IBeep slot 4 (a beep pattern; name unknown)
    });
    // ...
}

}  // namespace vota
