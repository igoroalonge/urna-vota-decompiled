// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/util/csystemdatetime.cpp (path inferred).
#include "api/util/csystemdatetime.h"

namespace api {

// wasm func 10855 (slot 0; observed executing during start-up)
// Emscripten's time() = Date.now() / 1000: UTC seconds, whereas CWasmSystemDateTime returns local time
// "expressed as if it were UTC" (see cdatetime.cpp). The two clocks differ by the browser's UTC offset.
std::time_t CSystemDateTime::GetDataHora() const
{
    return std::time(nullptr);
}

// wasm func 10853 (slot 1): EMPTY in this build (the function body is a bare `return`). On the urna this is
// where the system clock would be set (settimeofday / RTC); here setting the date through
// IAjusteDataHora (CAjusteDataHora, func 10874/10875) is silently ignored while CSystemDateTime is installed.  ?
void CSystemDateTime::SetDataHora(std::time_t /*instante*/)
{
}

} // namespace api
