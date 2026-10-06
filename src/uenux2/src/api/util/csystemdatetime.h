// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/util/csystemdatetime.h (path inferred; isystemdatetime.u19.cpp of unit u19
// already includes it).
//
// api::CSystemDateTime : api::ISystemDateTime   (typeinfo 1585260, vtable @1585244, 4 bytes)
// The default clock of the urna, created by the first ISystemDateTime::GetInst() (func 1155). ISystemDateTime
// declares its methods before the destructor:
//   [0] std::time_t GetDataHora() const   [1] void SetDataHora(std::time_t)   [2] dtor (ICF 174)  [3] deleting (144)
//
// WEB BUILD: it is used only during static initialisation - __wasm_call_ctors builds two static CDate objects
// (@1833312 and @1839068, through func 1382 = gmtime_r(GetDataHora())) - and until simulador::CSimuladorWasm
// replaces it with simulador::CWasmSystemDateTime (browser local time + offset). GetDataHora was observed
// executing at that time (runtime edge 1382 -> 10855); SetDataHora was never observed.
#pragma once

#include <ctime>

#include "api/util/isystemdatetime.h"

namespace api {

class CSystemDateTime : public ISystemDateTime {
public:
    std::time_t GetDataHora() const override;                        // wasm 10855
    void SetDataHora(std::time_t instante) override;                 // wasm 10853 (i64 by value)
};

} // namespace api
