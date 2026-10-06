// ecourna-lib/ecourna/api/io/cioerror.cpp   (path inferred; the function carries no srcloc of its own)
//
// Reconstructed from vota_web_wasm.wasm. Unit u12.
#include "ecourna/api/io/cioerror.hpp"

#include <array>
#include <cstring>
#include <string>

namespace ecourna::api::io {

namespace {
// Pointer table @1112652 (9 x const char*).
constexpr std::array<const char*, 9> NOMES_OPERACAO = {   // name inferred
    "None", "Open", "Close", "Read", "Write", "Seek", "Tell", "Sync", "Mode",
};
} // namespace

// inlined into func 3520 (the two copies of the lookup, one per branch below)
const char* Description(const EFileOperation op)
{
    const auto i = static_cast<unsigned>(op);
    if (i <= 8)
        return NOMES_OPERACAO[i];
    return "Description(EFileOperation) - não reconhecida";
}

// wasm func 3520
CIoError::CIoError(const EIoError code, const EFileOperation op, const std::string& msg, const int errnum,
                   const std::source_location& location)
    : CBaseError(code,
                 errnum != 0
                     ? msg + " [" + Description(op) + "] - " + std::to_string(errnum) + " - " + std::strerror(errnum)
                     : msg + " [" + Description(op) + "]",
                 location)                     // func 1143 = CBaseError<EIoError,...> ctor; then vptr = CIoError
{
    // std::to_string(int) is func 296 (to_chars + string(first,last)).
}

} // namespace ecourna::api::io
