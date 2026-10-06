// ecourna-lib/ecourna/api/util/datahora.cpp   (path and file name inferred, see datahora.hpp)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
#include "ecourna/api/util/datahora.hpp"

#include <format>
#include <string>

#include <boost/date_time/posix_time/time_parsers.hpp>

namespace ecourna::api::util {

// wasm func 1877
// Accepts either DataHoraJE ("YYYYMMDDThhmmss") or an ASN.1 GeneralizedTime in UTC ("YYYYMMDDhhmmssZ").
// For the second form the text is rebuilt as the first 8 characters + "T" + the next (up to) 6 characters;
// the 'Z' and any fraction are dropped and the value is NOT converted from UTC.
// A text that has both the 'T' and a final 'Z' ("YYYYMMDDThhmmssZ") becomes "YYYYMMDDTThhmms" and is rejected by
// the parser. Parse errors are Boost exceptions (bad_lexical_cast, gregorian::bad_day_of_month, ...), not CError.
boost::posix_time::ptime ConverteDataHoraJE(const std::string& dataHora)
{
    std::string texto = dataHora;
    if (texto.size() > 8 && texto.back() == 'Z') {                  // size >= 9 and last char == 'Z' (90)
        texto = texto.substr(0, 8) + "T" + texto.substr(8, 6);     // "T" = literal @323488
    }
    return boost::date_time::parse_iso_time<boost::posix_time::ptime>(texto, 'T');   // func 5851, sep 84
}

// wasm func 9220
// The body (api_f6155) converts the 64-bit microsecond count to a civil date (Julian-day arithmetic) and
// time of day; special values (not_a_date_time, +/-infinity) are mapped to fixed day numbers first. The date
// is recomputed for year, month and day separately (three date() calls). The YEAR goes through
// std::to_string(int) (ecourna_f296) + std::stoi(s, nullptr, 10) (invoke slot 6201); month and day are the plain
// 16-bit fields. Because 6155 is a merge-similar body, api::FormataAAAAMMDDhhmmss (2685) has the same
// round trip in its source.                                                  // ? exact spelling of the round trip
std::string FormataDataHoraJE(const boost::posix_time::ptime& dataHora)
{
    const int ano = std::stoi(std::to_string(dataHora.date().year()));
    const auto hora = dataHora.time_of_day();
    return std::format("{:04}{:02}{:02}T{:02}{:02}{:02}",
                       ano, static_cast<int>(dataHora.date().month()), static_cast<int>(dataHora.date().day()),
                       hora.hours(), hora.minutes(), hora.seconds());
}

} // namespace ecourna::api::util
