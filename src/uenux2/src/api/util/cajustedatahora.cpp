// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original file UNKNOWN - path inferred: uenux2/src/api/util/cajustedatahora.cpp
// (the analysis tools filed these two functions under md/estadoaplicacao/cajustedatahora.cpp because
//  of the class name; this is the unrelated api-level class api::CAjusteDataHora : api::IAjusteDataHora,
//  vtable @1584164, a CPolySingletonList-registered service).
//
// IAjusteDataHora slot order: 0 AjustaDataHora(const CDateTime&), 1 AjustaDataHora(time_t),
// 2 ~dtor, 3 deleting dtor (the interface declares its methods before the virtual destructor).
//
// Web build: ISystemDateTime is implemented by simulador::CWasmSystemDateTime, whose slot 1 does not
// touch any clock - it stores `novo - js_obter_data_hora_local_navegador()` as an offset that is added
// to the browser's local time afterwards (func 7945). On the urna (api::CSystemDateTime, func 10853)
// the same slot is an empty stub in this build.
#include "api/util/cajustedatahora.h"

#include "api/util/isystemdatetime.h"

namespace api {

// wasm func 10875 (vtable slot 0)  name inferred
void CAjusteDataHora::AjustaDataHora(const CDateTime& dataHora)
{
    ISystemDateTime::GetInst().SetDataHora(dataHora.ToTimeT());   // vota_f1381 = timegm(...)
}

// wasm func 10874 (vtable slot 1)  name inferred
void CAjusteDataHora::AjustaDataHora(const std::time_t& instante)
{
    ISystemDateTime::GetInst().SetDataHora(instante);
}

}  // namespace api
