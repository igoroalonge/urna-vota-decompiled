// ecourna-lib/ecourna/api/util/cstringutils.hpp   (path inferred from the .cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// ecourna::api::util::CStringUtils: static helpers only (no RTTI, no vtable). Do not confuse it with
// uenux2's api::CStringUtils (uenux2/src/api/util/cstringutils.cpp: GetVersionNumber, SetBit...) or with
// comum's util::CStringUtil.
//
// Error type: exception::CBaseError<util::EUtilError, SErrorLimits{1875, 1900}> (typeinfo @1117592,
// vtable @1117872, constructed through the thunk wasm func 9410 -> func 1011).
#pragma once

#include <cstdint>
#include <source_location>
#include <string>
#include <vector>

#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/types.hpp"   // uebyte, ueword, uedword, ueqword, ueint16, ueint32

namespace ecourna::api::util {

enum class EUtilError : int {             // values certain, names inferred
    ValorInvalido          = 1875,        // empty (after trim) or a character outside the allowed set
    ValorComSobra          = 1876,        // strtoull/strtoll did not consume the whole string
    ValorForaDoIntervalo   = 1877,        // > max (or < min), or errno == ERANGE
    HexadecimalInvalido    = 1878,        // HexStringToBytes: a non-hex character
    // 1879..1881 not used in this binary
    BitArraySemLeitura     = 1882,        // CBitArray::GetValor(uebyte)   (cbitarray.cpp:44)
    BitArrayNaoAlocado     = 1883,        // CBitArray::GetValor(ueqword, uebyte) (cbitarray.cpp:56)
    BitArrayPosicaoInvalida= 1884,        // CBitArray::GetValor(ueqword, uebyte) (cbitarray.cpp:64)
    // 1889: csynchronizer.cpp:59 (inlined into ecourna::api::io::CFile::Sync) - not in u13
};
using CUtilError = exception::CBaseError<EUtilError, exception::SErrorLimits{1875, 1900}>;   // alias name inferred

class CStringUtils {
public:
    // ---- numeric conversions (u13) ----------------------------------------------------------------
    static uebyte   ToByte(const std::string& valor);      // wasm func 1142 (line 714)  -> body func 6153
    static ueint16  ToInt16(const std::string& valor);     // wasm func 3508 (line 719)
    static ueword   ToWord(const std::string& valor);      // wasm func 2201 (line 724)  -> body func 6153
    static ueint32  ToInt32(const std::string& valor);     // wasm func 2200 (line 729)
    static ueqword  ToQWord(const std::string& valor);     // wasm func 1141 (line 734)
    static uedword  ToDWord(const std::string& valor);     // wasm func 1523 (line 739)
    static uedword  HexToInt(const std::string& valor);    // wasm func 3507 (line 744)
    static ueqword  HexToQWord(const std::string& valor);  // line 754; exists only inlined (in func 3700)
    static std::vector<uebyte> HexStringToBytes(const std::string& valor);   // wasm func 3506 (line 808)

    // ---- text transforms ------------------------------------------------------------------------
    static std::string ToLower(const std::string& texto);  // wasm func 1879 (u13)             name inferred
    static void ToLower(std::string& texto);               // wasm func 5158 (not in u13)      name inferred
    static std::string ToUpper(const std::string& texto);  // wasm func 5156 (not in u13)      name inferred
    static void ToUpper(std::string& texto);               // wasm func 3509 (not in u13)      name inferred
    static std::string Trim(const std::string& texto);     // wasm func 1374 (not in u13)      name inferred
    static void Trim(std::string& texto);                  // wasm func 9406 (u13)             name inferred
    static void Replace(std::string& texto, char de, const std::string& para);   // wasm func 9398 (not in u13) name inferred

private:
    // Shared bodies of the numeric conversions (no srcloc of their own: they throw with the caller's
    // std::source_location, received as a defaulted argument - the srclocs of ToByte... all have column
    // 12, the column of the call in "    return ToUnsigned(").                              names inferred
    static ueqword ToUnsigned(const std::string& valor, int base, const char* caracteresValidos, ueqword maximo,
                              const std::source_location& local = std::source_location::current());   // wasm func 2202
    static std::int64_t ToSigned(const std::string& valor, const char* caracteresValidos,
                                 std::int64_t minimo, std::int64_t maximo,
                                 const std::source_location& local = std::source_location::current()); // wasm func 5155
};

} // namespace ecourna::api::util
