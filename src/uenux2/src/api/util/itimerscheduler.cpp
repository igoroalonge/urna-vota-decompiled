// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/api/util/itimerscheduler.cpp (srcloc itimerscheduler.cpp:23).
#include "api/util/itimerscheduler.h"

#include "api/util/ctimerscheduler.h"

namespace api {

// wasm func 1259 - observed executing (every screen with a clock / blinking field)
ITimerScheduler& ITimerScheduler::GetInst()
{
    if (!CPolySingletonList::exists<ITimerScheduler>())                // api_f1654
        CreateInst<CTimerScheduler>();                                 // func 5444
    return CPolySingletonList::instance<ITimerScheduler>();            // srcloc :23 passed to instance()
}

} // namespace api
