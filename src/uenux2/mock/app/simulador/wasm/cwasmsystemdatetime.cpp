// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmsystemdatetime.cpp
//
// simulador::CWasmSystemDateTime : api::ISystemDateTime - the urna's clock in the web build.
// RTTI typeinfo @1532068; vtable @1532052: [0] GetDataHora (7954) [1] SetDataHora (7945) [2] ~ (ICF 174)
// [3] deleting (ICF 144): ISystemDateTime declares its two methods BEFORE the virtual destructor.
// The start-up (func 8302 -> push 4890) REPLACES the default api::CSystemDateTime (time(0) / empty setter)
// registered by the first ISystemDateTime::GetInst() (func 1155).
//
// js_obter_data_hora_local_navegador() ("get the browser's local date-time") returns
//     Date.getTime()/1000 - getTimezoneOffset()*60
// i.e. the LOCAL wall-clock time encoded as if it were a UTC epoch. api::CDateTime::ConvertFromLocalTime
// (func 5476) decodes it with gmtime_r, so the urna displays the browser's local time.
//
// Setting the clock (api::CAjusteDataHora -> SetDataHora, e.g. vota::CIniciodeCiclo::AjustaDataHora moving
// the clock to the configured start of the election) never touches any real clock: it only records an
// offset from the browser time, which later readings add back.
#include <cstdint>
#include <ctime>

#include "api/util/isystemdatetime.h"
#include "simulador/wasm/cwasmjs.h"   // double js_obter_data_hora_local_navegador()

namespace simulador {

class CWasmSystemDateTime : public api::ISystemDateTime {
public:
    std::time_t GetDataHora() override;                  // slot 0 (7954)   name inferred
    void SetDataHora(std::time_t instante) override;     // slot 1 (7945)   name as in u05 (cajustedatahora.cpp)

private:
    std::int64_t m_deslocamento = 0;                     // +8  seconds added to the browser clock (16-byte object)
};

// wasm func 7954 - slot 0. Observed executing (every CDateTime::Now()).
std::time_t CWasmSystemDateTime::GetDataHora()
{
    return m_deslocamento + static_cast<std::int64_t>(js_obter_data_hora_local_navegador());   // saturating trunc
}

// wasm func 7945 - slot 1
void CWasmSystemDateTime::SetDataHora(std::time_t instante)
{
    m_deslocamento = instante - static_cast<std::int64_t>(js_obter_data_hora_local_navegador());
}

}  // namespace simulador
