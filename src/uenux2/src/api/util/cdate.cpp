// Reconstructed from vota_web_wasm.wasm (unit u20, owner of this file).
// Original: uenux2/src/api/util/cdate.cpp (srclocs cdate.cpp:94, :104, :130).
// Other fragments of this file: cdate.u13.cpp (CDate::IsValid, func 5479) and
// src/uenux2/src/app/comum/dados/u05-foreign-fragments.cpp (operator+=/-=, funcs 5477/5478/3648).
//
// api::CDate is an 8-byte value type:
//   +0 ueword m_dia   +2 ueword m_mes   +4 ueword m_ano   +6 ueword m_diaSemana (0 = domingo, from strptime)
#include "api/util/cdate.h"

#include <ctime>
#include <format>
#include <string>

#include "ecourna/api/util/cstringutils.hpp"

namespace api {

// ueword @1584256 (shared with CDate::IsValid). NOT a compile-time constant: both 3649 and IsValid (5479)
// load it from memory (i32.load16_u 1584256), so it is an externally visible variable (a non-const global
// or a static member whose initializer the optimizer could not see).              kind/name inferred
ueword SECULO = 2000;

namespace {
// const char* table @1584308
constexpr const char* DIAS_SEMANA[7] = {"DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB"};

// "YYYY" / "YY": inlined string::insert(0, n - size, '0') (func 2130 = std::string::insert(pos, n, c)).
// "DD" / "MM" are padded differently in the binary: when size() <= 1 a new 2-byte string "0" + s is
// built (same result for these non-negative values).
std::string PadEsquerda(std::string s, std::size_t n)
{
    if (s.size() < n)
        s.insert(0, n - s.size(), '0');
    return s;
}
} // namespace

// wasm func 3649 - observed executing                               srclocs cdate.cpp:94 and :104
// Accepts "DDMMAA" or "DDMMAAAA" (see IsValid); a 2-digit year gets SECULO added.
CDate::CDate(const std::string& data)
    : m_dia(0), m_mes(0), m_ano(0), m_diaSemana(0)
{
    if (!IsValid(data))                                                      // func 5479
        throw CUeUtilError(EUeUtilError{7001}, "Data inválida [" + data + "]");

    using ecourna::api::util::CStringUtils;
    const ueint16 ano = CStringUtils::ToInt16(data.substr(4));               // func 3508
    const std::string completa = data.substr(0, 4)
        + std::to_string(static_cast<ueint16>((data.size() == 6 ? SECULO : 0) + ano));   // ecourna_f296

    std::tm tm{};
    if (!::strptime(completa.c_str(), "%d%m%Y", &tm))        // env.strptime: implemented in the JS glue
        throw CUeUtilError(EUeUtilError{7002}, "Data inválida [" + completa + "]");
    m_ano = static_cast<ueword>(tm.tm_year + 1900);
    m_mes = static_cast<ueword>(tm.tm_mon + 1);
    m_dia = static_cast<ueword>(tm.tm_mday);
    m_diaSemana = static_cast<ueword>(tm.tm_wday);   // filled by the glue's JS strptime (new Date().getDay());
                                                     // musl's strptime would leave the zeroed 0 (= "DOM")
}

// wasm func 2765 (tools: api_f2765) - observed executing. Delegating constructor.        name inferred
// Callers: CDateTime(const std::string&) (default 01/01/2000), CDateTime::ConvertFromLocalTime,
// CDateTime += seconds (func 2233), comum_f2276.
// Parameter types: the packed std::format arg types are 3270 = {unsigned, unsigned, int}, and
// ConvertFromLocalTime passes tm_mday / (tm_mon + 1) & 255 as bytes and the year sign-extended from 16 bits.
CDate::CDate(uebyte dia, uebyte mes, ueint16 ano)
    : CDate(std::format("{:02}{:02}{:04}", dia, mes, ano))
{
}

// wasm func 706 - observed executing (BU, QR code, zerésima, CTradutorFrase "<DT...>")  srcloc cdate.cpp:130
// Tokens: "A" weekday abbreviation; "DD"/"D" day (zero-padded / plain); "MM"/"M" month;
// "YYYY" year padded to 4; "YY" year % 100 padded to 2; a lone "Y" year % 100 unpadded.
// Every other character is copied.
std::string CDate::Format(const std::string& formato) const
{
    std::string r;
    for (std::size_t i = 0; i < formato.size(); ++i) {
        const char c = formato[i];
        switch (c) {
        case 'A':
            if (m_diaSemana >= 7)
                throw CUeUtilError(EUeUtilError{7003},
                                   std::format("Dia da semana inválido {}", static_cast<short>(m_diaSemana)));
            r += DIAS_SEMANA[m_diaSemana];
            break;
        case 'D':
            if (i + 1 < formato.size() && formato[i + 1] == 'D') {
                r += PadEsquerda(std::to_string(static_cast<short>(m_dia)), 2);
                ++i;
            } else {
                r += std::to_string(static_cast<short>(m_dia));
            }
            break;
        case 'M':
            if (i + 1 < formato.size() && formato[i + 1] == 'M') {
                r += PadEsquerda(std::to_string(static_cast<short>(m_mes)), 2);
                ++i;
            } else {
                r += std::to_string(static_cast<short>(m_mes));
            }
            break;
        case 'Y':
            if (i + 3 < formato.size() && formato.substr(i, 4) == "YYYY") {
                r += PadEsquerda(std::to_string(static_cast<short>(m_ano)), 4);
                i += 3;
            } else if (i + 1 < formato.size() && formato.substr(i, 2) == "YY") {
                r += PadEsquerda(std::to_string(static_cast<short>(m_ano) % 100), 2);
                i += 1;
            } else {
                r += std::to_string(static_cast<short>(m_ano) % 100);
            }
            break;
        default:
            r.push_back(c);
            break;
        }
    }
    return r;
}

} // namespace api
