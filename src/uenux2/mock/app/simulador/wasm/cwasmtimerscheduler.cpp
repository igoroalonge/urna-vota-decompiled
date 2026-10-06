// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmtimerscheduler.cpp
//
// simulador::CWasmTimerScheduler : api::ITimerScheduler (RTTI typeinfo @1532192 / @1531264; vtable @1532180:
// [0] CreateTimer (7871), [1] ~ (ICF 174), [2] deleting (ICF 144); a 4-byte object). Registered first of
// all platform services by func 8302 through ITimerScheduler::CreateInst<CWasmTimerScheduler>()
// (itimerscheduler.h:40), which is why ITimerScheduler::GetInst() never builds the urna's api::CTimerScheduler.
#include "simulador/wasm/cwasmtimer.h"

#include <chrono>
#include <functional>
#include <memory>

#include "api/util/itimerscheduler.h"

namespace simulador {

class CWasmTimerScheduler : public api::ITimerScheduler {
public:
    std::shared_ptr<api::ITimer> CreateTimer(const std::chrono::milliseconds& intervalo,
                                             const std::function<void()>& callback) override;
};

// wasm func 7871 - slot 0. Observed executing. The timer is returned STOPPED: the caller (a GUI field)
// calls Start() when its form becomes active.
std::shared_ptr<api::ITimer> CWasmTimerScheduler::CreateTimer(const std::chrono::milliseconds& intervalo,
                                                              const std::function<void()>& callback)
{
    return std::make_shared<CWasmTimer>(intervalo, callback);   // the std::function is copied (__clone)
}

}  // namespace simulador
