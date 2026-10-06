// Reconstructed from vota_web_wasm.wasm (unit u20, owner of this file).
// Original: uenux2/src/api/util/cdatetime.cpp (srclocs cdatetime.cpp:31, :61, :206).
// Other fragment: cdatetime.u02.cpp (FormataDataHora / FormataAAAAMMDDhhmmss, funcs 6155 / 2685).
//
// api::CDateTime (12 bytes) = +0 CDate m_data (8 bytes) + 8 CTime m_hora (seconds since midnight).
// The urna keeps "local" civil time; it is converted to/from time_t with gmtime_r/timegm, i.e. the
// time_t is "local time expressed as if it were UTC" (the web mock CWasmSystemDateTime returns the
// browser's local time in that form).
#include "api/util/cdatetime.h"

#include <ctime>
#include <format>
#include <string>

namespace api {

namespace {
// Inlined into func 5475. "DDMMAAAAhhmm" or "DDMMAAAAhhmmss", digits only.          name inferred
bool IsValidDateTime(const std::string& texto)
{
    if (texto.size() != 12 && texto.size() != 14)
        return false;
    if (texto.find_first_not_of("0123456789") != std::string::npos)
        return false;
    return CDate::IsValid(texto.substr(0, 8)) && CTime::IsValid(texto.substr(8));   // funcs 5479 / 5449
}
} // namespace

// wasm func 5475 - observed executing (reads of every DataHoraJE)            srcloc cdatetime.cpp:31
CDateTime::CDateTime(const std::string& dataHora)
    : m_data(1, 1, 2000)                                    // func 2765
    , m_hora()                                              // func 2230
{
    if (!IsValidDateTime(dataHora))
        throw CUeUtilError(EUeUtilError{7005}, "Data inválida [" + dataHora + "]");
    m_data = CDate(dataHora.substr(0, 8));                  // func 3649
    m_hora = CTime(dataHora.substr(8));                     // func 3643
}

// wasm func 5476 - observed executing (CPreZeresima, api_f1000 = "now")     srcloc cdatetime.cpp:61
void CDateTime::ConvertFromLocalTime(std::time_t instante)
{
    std::tm tm;
    if (!::gmtime_r(&instante, &tm))                        // gmtime, not localtime: see file comment
        throw CUeUtilError(EUeUtilError{7006}, "Data inválida [" + std::to_string(instante) + "]");
    m_hora = CTime(static_cast<uebyte>(tm.tm_hour), static_cast<uebyte>(tm.tm_min),
                   static_cast<uebyte>(tm.tm_sec));        // func 3642
    m_data = CDate(static_cast<uebyte>(tm.tm_mday), static_cast<uebyte>(tm.tm_mon + 1),
                   static_cast<ueint16>(tm.tm_year + 1900)); // func 2765 (uebyte, uebyte, ueint16)
}

// wasm func 2764 - observed executing (only caller: vota::CInformacaoEleitor::Inicializar, func 7787, i.e.
// votaInit)  srcloc :206
// "AAAAMMDDhhmmss" -> CDateTime("DDMMAAAAhhmmss").
CDateTime CDateTime::ConvertFromTimestamp(const std::string& timestamp)
{
    if (timestamp.size() != 14)
        throw CUeUtilError(EUeUtilError{7008},
            std::format("Data/hora [{}] não está no formato 'AAAAMMDDhhmmss'", timestamp));
    return CDateTime(timestamp.substr(6, 2) + timestamp.substr(4, 2) + timestamp.substr(0, 4)
                     + timestamp.substr(8));
}

} // namespace api
