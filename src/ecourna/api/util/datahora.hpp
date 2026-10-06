// ecourna-lib/ecourna/api/util/datahora.hpp   (path and file name inferred; units u11/u14 already include this
//   name. The two helpers have no srcloc and no class; they may equally live in a header of
//   ecourna/app/dados/asn/. Names inferred.)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// ASN.1 "DataHoraJE" (ModuloTiposEleitorais, a GeneralString) <-> boost::posix_time::ptime.
// DataHoraJE text: "YYYYMMDDThhmmss" (ISO 8601 basic format, local time, no zone).
#pragma once

#include <string>

#include <boost/date_time/posix_time/ptime.hpp>

namespace ecourna::api::util {

// wasm func 1877 (table slot 6735). Observed executing (CConversorHorariosUrna and the other converters that
// read dates from the election files at votaInit).
boost::posix_time::ptime ConverteDataHoraJE(const std::string& dataHora);

// wasm func 9220 (table slot 6686): thunk into the shared formatter body api_f6155 (merge-similar-functions
// with api::FormataAAAAMMDDhhmmss, func 2685), passing the format string [begin, end) @8913..8944.
std::string FormataDataHoraJE(const boost::posix_time::ptime& dataHora);

} // namespace ecourna::api::util
