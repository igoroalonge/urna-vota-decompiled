// ecourna-lib/ecourna/api/pattern/cbasetype.hpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/pattern/cbasetype.hpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u12.
//
// CBaseType<TYPE, MIN_VAL, MAX_VAL, BASIC_TYPE> is a range-checked value type. ecourna's electoral id types
// (zone, section, municipality, office, party ...) are typedefs of it, declared in
// ecourna/app/dados/tiposbasicos.hpp (not in this unit). The object holds only the value (2 or 4 bytes).
// Building it with an out-of-range value throws
//     CPatternError(1300, "O tipo '<name>' deve ter valores no intervalo [<MIN>,<MAX>].")   (line 39)
// The type name comes from a static std::string that is selected by BASIC_TYPE (a TypeName<> trait,
// name inferred).
//
// Instantiations present in the binary (all srcloc cbasetype.hpp:39):
//
//   func  TYPE / MIN / MAX / BASIC_TYPE                  type name (static std::string @addr)
//   5682  unsigned short, 1, 99, 3                      "CargoID"           @1122284   (inlined body)
//   9215  unsigned short, 0, 99, 4                      "PartidoID"         @1122296   (inlined body)
//   2851  unsigned int,   0, 99999, 6                   "MunicipioID"       @1912004   (-> merged body 6042)
//   5850  unsigned short, 0, 9999, 7                    "ZonaID"            @1122308   (-> merged body 2299)
//   5849  unsigned int,   0, 9999, 8                    "LocalID"           @1122320   (-> merged body 6042)
//   5848  unsigned short, 0, 9999, 9                    "SecaoID"           @1122332   (-> merged body 2299)
//   1960  unsigned short, 0, 9999, 15                   "QtdEleitor"        @1122344   (-> merged body 2299)
//   2277  unsigned short, 0, 9999, 36                   "Ano"               @1122356   (-> merged body 2299)
//   2850  unsigned short, 0, 999, 38                    "ScoreHabilitacao"  @1912184   (-> merged body 2299)
//   2666  ETipoIdentificadorEleitor, TipoIDPrimeiro(1),
//         TipoIDNumeroLivre(3), 40                      "TipoIdentificadorEleitor" @1912208 (inlined body)
//
// The names longer than 10 characters (MunicipioID, ScoreHabilitacao, TipoIdentificadorEleitor) do not fit
// libc++'s short-string buffer. They live in .bss and are built by __wasm_call_ctors (func 14478). The short
// ones are constant-initialised in .data.
//
// wasm-opt "merge-similar-functions" folded the instantiations that differ only in constants into two
// shared bodies. Each constructor above is a 35/37-byte thunk:
//   func 2299  body for 16-bit TYPE with MIN_VAL == 0:  (this, value, &srcloc, &name.data, &name.size,
//                                                        &name.flags, MAX_VAL, MAX_VAL + 1)
//   func 6042  the same body for 32-bit TYPE
// Because MIN_VAL is 0 and TYPE is unsigned, only `value >= MAX_VAL + 1` is tested.
#pragma once

#include <format>
#include <source_location>
#include <string>

#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/api/pattern/epatternerror.hpp"   // EPatternError; CPatternError = CBaseError<EPatternError, ...> (not in this unit)

namespace ecourna::api::pattern {

// Name used in the error message, one static std::string per BASIC_TYPE id (see the table above).
template <int BASIC_TYPE>
struct TypeName;                                                   // name inferred

template <typename TYPE, TYPE MIN_VAL, TYPE MAX_VAL, int BASIC_TYPE>
class CBaseType {
public:
    // wasm funcs 1960, 2277, 2666, 2850, 2851, 5682, 5848, 5849, 5850, 9215 (+ merged bodies 2299, 6042)
    CBaseType(TYPE value)                                          // srcloc line 39 (`this` is returned)
        : m_value(value)                                           // stored BEFORE the check
    {
        if (value < MIN_VAL || value > MAX_VAL) {
            throw CPatternError(EPatternError(1300),               // enumerator name unknown
                                std::format("O tipo '{}' deve ter valores no intervalo [{},{}].",
                                            TypeName<BASIC_TYPE>::value, MIN_VAL, MAX_VAL));   // line 39
        }
    }

    operator TYPE() const { return m_value; }                      // ? (callers read the field directly)

private:
    TYPE m_value;   // +0
};

} // namespace ecourna::api::pattern
