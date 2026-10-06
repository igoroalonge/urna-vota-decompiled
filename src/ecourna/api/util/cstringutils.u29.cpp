// FRAGMENT of ecourna-lib/ecourna/api/util/cstringutils.cpp reconstructed by unit u29 from vota_web_wasm.wasm.
// (The class is reconstructed by unit u13, src/ecourna/api/util/cstringutils.cpp.)
#include "ecourna/api/util/cstringutils.hpp"

namespace ecourna::api::util {

// wasm func 5156 (table slot 29): copy, then the in-place, Latin-1-aware upper-caser ToUpper(std::string&)
// (func 3509, reached through table slot 2920; the copy-and-transform body is shared with ToLower, merged
// func 3942). Callers: votaInit (the "uf" option before it goes into eg.bin) and api_f6592.
std::string CStringUtils::ToUpper(const std::string& texto)
{
    std::string resultado = texto;
    ToUpper(resultado);
    return resultado;
}

}  // namespace ecourna::api::util
