// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/util/ctime.cpp (attested; owner unit u20, which calls this function
// "CTime::IsValid, func 5449, other unit").
#include <algorithm>
#include <string>

#include "api/util/ctime.h"
#include "ecourna/api/util/cstringutils.hpp"

namespace api {

// wasm func 5449 (tools: api_f5449) - observed executing (every date/time read from the election data).
// Callers: CTime::CTime(const std::string&) (3643; its srcloc ctime.cpp:63 is the inlined anonymous
// EncodeTime(const std::string&), see ctime.cpp) and CDateTime::CDateTime(const std::string&)
// (5475). Accepts "hhmm" or "hhmmss": 4 or 6 decimal digits, hh < 24, mm < 60, ss < 60.       name inferred
bool CTime::IsValid(const std::string& hora)
{
    if (hora.size() != 4 && hora.size() != 6)
        return false;
    // bit test against the mask 0x03FF000000000000 = the characters '0'..'9'
    if (!std::ranges::all_of(hora, [](char c) { return c >= '0' && c <= '9'; }))
        return false;

    using ecourna::api::util::CStringUtils;
    const uebyte horas = CStringUtils::ToByte(hora.substr(0, 2));                    // func 1142
    const uebyte minutos = CStringUtils::ToByte(hora.substr(2, 2));                  // substr: out_of_range check kept
    const bool segundosValidos = hora.size() != 6 || CStringUtils::ToByte(hora.substr(4, 2)) < 60;
    return horas < 24 && minutos < 60 && segundosValidos;
}

}  // namespace api
