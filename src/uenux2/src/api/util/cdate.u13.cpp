// uenux2/src/api/util/cdate.cpp  --  FRAGMENT written by unit u13 (file known from the srclocs of
// api::CDate::CDate(const std::string&), func 3649, lines 94/104, which calls this function).
#include "api/util/cdate.h"

#include <algorithm>
#include <string>

#include "ecourna/api/util/cstringutils.hpp"

namespace api {

// SECULO (declared in the owner file cdate.cpp): ueword 2000 @1584256, added to 2-digit years. Both this
// function and CDate::CDate(const std::string&) (func 3649) LOAD it from memory (i32.load16_u 1584256) instead
// of using an immediate, so in the original it is NOT a compile-time constant: a variable (a non-const global or
// a static member whose initializer is not visible). It sits just before the cdate.cpp srcloc records.
// (cdate.cpp currently writes it as constexpr, which would have been folded.)               name/kind inferred

namespace {

// inlined                                                                                        name inferred
unsigned DiasNoMes(unsigned mes, int ano)
{
    switch (mes) {
    case 1: case 3: case 5: case 7: case 8: case 10: case 12:     // bitmask 5546
        return 31;
    case 2:
        if (ano % 4 != 0) return 28;
        if (ano % 100 != 0) return 29;
        return (ano % 400 != 0) ? 28 : 29;
    default:
        return 30;
    }
}
} // namespace

// wasm func 5479 - name inferred. Observed executing (CDate/CDateTime construction).
// Accepts "DDMMAA" or "DDMMAAAA", digits only, with a valid Gregorian day/month.
bool CDate::IsValid(const std::string& data)
{
    if (data.size() != 6 && data.size() != 8)
        return false;
    if (!std::all_of(data.begin(), data.end(), [](char c) { return c >= '0' && c <= '9'; }))
        return false;

    using ecourna::api::util::CStringUtils;
    const uebyte dia = CStringUtils::ToByte(data.substr(0, 2));
    const uebyte mes = CStringUtils::ToByte(data.substr(2, 2));
    const ueint16 ano = CStringUtils::ToInt16(data.substr(4));
    if (dia == 0 || mes < 1 || mes > 12)
        return false;
    const int anoCompleto = static_cast<ueint16>((data.size() == 6 ? SECULO : 0) + ano);
    return dia <= DiasNoMes(mes, anoCompleto);
}

} // namespace api
