// uenux2/src/app/comum/asn/util.cpp  --  FRAGMENT written by unit u13 (file known from the srclocs of
// the other comum::asn::Utils functions, e.g. DesconverteDataHoraJE line 41, which calls this one).
#include "comum/asn/util.h"

#include <string>

#include "api/util/cdate.h"
#include "ecourna/api/util/cstringutils.hpp"

namespace comum::asn {

// wasm func 2276 - name as used by u03 (cconversorestadogeralgap.cpp). Observed executing.
// ModuloTiposEleitorais::DataJE is NumericString(SIZE(8)) "YYYYMMDD" (the std::string sits at +8 of the ASN.1
// object). The three substrings are built first (year, month, day); a string shorter than 6 characters throws
// std::out_of_range from substr, one of exactly 6 characters has an empty day and throws EUtilError 1875 from
// ToByte (evaluated first), and one of 7 characters gives a one-digit day that ToByte ACCEPTS.
// api::CDate(dia, mes, ano) is func 2765: it formats "{:02}{:02}{:04}" and parses it again with strptime "%d%m%Y".
api::CDate Utils::DesconverteDataJE(const ModuloTiposEleitorais::DataJE& data)
{
    using ecourna::api::util::CStringUtils;
    const std::string& texto = data.value();   // the std::string at +8 of the ASN.1 NumericString   // ?accessor name
    const std::string ano = texto.substr(0, 4);
    const std::string mes = texto.substr(4, 2);
    const std::string dia = texto.substr(6, 2);
    return api::CDate(CStringUtils::ToByte(dia), CStringUtils::ToByte(mes),
                      static_cast<ueint16>(CStringUtils::ToWord(ano)));
}

} // namespace comum::asn
