// ecourna-lib/ecourna/api/util/cstringutils.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/util/cstringutils.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13. Only the functions of unit u13 are written out; the
// neighbouring members that other units own are listed at the end.
//
// Library code the unit builder put here: wasm func 2965 = musl isxdigit(c) ("c-'0' < 10 || (c|32)-'a' < 6"),
// called by HexStringToBytes and by libc++'s num_put. Not reconstructed.
//
// errno: musl's errno lives at @1931660 and ERANGE is 68 (Emscripten uses the WASI errno numbering).
#include "ecourna/api/util/cstringutils.hpp"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <source_location>
#include <string>
#include <vector>

namespace ecourna::api::util {

// The allowed-character sets are LOCAL arrays: every wrapper (and the merged body 6153) copies the literal
// onto its stack frame (13 bytes from @339963 "+-0123456789", 25 bytes from @1118128
// "xXabcdefABCDEF0123456789", inside the .rodata block that starts with the Base64 alphabet @1112848) and
// passes the copy's address. A namespace-scope constant would have been passed by address, without a copy.
// Signs are accepted by the character check; strtoull then NEGATES "-n" modulo 2^64. Names inferred.
#define CARACTERES_DECIMAIS_LITERAL     "+-0123456789"               // reconstruction helper
#define CARACTERES_HEXADECIMAIS_LITERAL "xXabcdefABCDEF0123456789"   // reconstruction helper

namespace {

// Value of one hex digit, -1 otherwise (inlined in HexStringToBytes).                                     name inferred
int ValorHexa(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}
} // namespace

// ------------------------------------------------------------------------------------------------------
// Shared conversion bodies
// ------------------------------------------------------------------------------------------------------

// wasm func 2202 - name inferred. Observed executing.
// The message quotes the ORIGINAL argument (before trimming).
ueqword CStringUtils::ToUnsigned(const std::string& valor, int base, const char* caracteresValidos, ueqword maximo,
                                 const std::source_location& local)
{
    std::string texto = valor;
    Trim(texto);                                                            // func 9406
    if (texto.empty() || texto.find_first_not_of(caracteresValidos) != std::string::npos)
        throw CUtilError(EUtilError::ValorInvalido, "valor invalido [" + valor + "]", local);

    errno = 0;
    char* fim = nullptr;
    const ueqword numero = std::strtoull(texto.c_str(), &fim, base);
    if (*fim != '\0')
        throw CUtilError(EUtilError::ValorComSobra, "valor invalido [" + valor + "]", local);
    if (numero > maximo || errno == ERANGE)                                 // unsigned compare (i64.ge_u)
        throw CUtilError(EUtilError::ValorForaDoIntervalo, "valor invalido [" + valor + "]", local);
    return numero;
}

// wasm func 5155 - name inferred. Observed executing (ToInt16, from api::CDate).
std::int64_t CStringUtils::ToSigned(const std::string& valor, const char* caracteresValidos,
                                    std::int64_t minimo, std::int64_t maximo, const std::source_location& local)
{
    std::string texto = valor;
    Trim(texto);
    if (texto.empty() || texto.find_first_not_of(caracteresValidos) != std::string::npos)
        throw CUtilError(EUtilError::ValorInvalido, "valor invalido [" + valor + "]", local);

    errno = 0;
    char* fim = nullptr;
    const std::int64_t numero = std::strtoll(texto.c_str(), &fim, 10);
    if (*fim != '\0')
        throw CUtilError(EUtilError::ValorComSobra, "valor invalido [" + valor + "]", local);
    if (numero > maximo || numero < minimo || errno == ERANGE)
        throw CUtilError(EUtilError::ValorForaDoIntervalo, "valor invalido [" + valor + "]", local);
    return numero;
}

// ------------------------------------------------------------------------------------------------------
// Public conversions
// ------------------------------------------------------------------------------------------------------

// wasm func 1142 (srcloc line 714). Observed executing.
// ToByte and ToWord differ only in constants, so wasm-opt merged their bodies into func 6153
// ("ToUnsigned(valor, 10, caracteresValidos, max, loc) & mask", with its own stack copy of the set),
// called with (255, 255) and (65535, 65535).
uebyte CStringUtils::ToByte(const std::string& valor)
{
    const char caracteresValidos[] = CARACTERES_DECIMAIS_LITERAL;
    return ToUnsigned(valor, 10, caracteresValidos, std::numeric_limits<uebyte>::max());
}

// wasm func 3508 (srcloc line 719). Observed executing. The result is sign-extended from 16 bits.
ueint16 CStringUtils::ToInt16(const std::string& valor)
{
    const char caracteresValidos[] = CARACTERES_DECIMAIS_LITERAL;
    return ToSigned(valor, caracteresValidos, std::numeric_limits<ueint16>::min(), std::numeric_limits<ueint16>::max());
}

// wasm func 2201 (srcloc line 724) -> func 6153. Observed executing.
ueword CStringUtils::ToWord(const std::string& valor)
{
    const char caracteresValidos[] = CARACTERES_DECIMAIS_LITERAL;
    return ToUnsigned(valor, 10, caracteresValidos, std::numeric_limits<ueword>::max());
}

// wasm func 2200 (srcloc line 729)
ueint32 CStringUtils::ToInt32(const std::string& valor)
{
    const char caracteresValidos[] = CARACTERES_DECIMAIS_LITERAL;
    return ToSigned(valor, caracteresValidos, std::numeric_limits<ueint32>::min(), std::numeric_limits<ueint32>::max());
}

// wasm func 1141 (srcloc line 734). maximo = UINT64_MAX, so only ERANGE can fail the range test and
// "-5" is accepted as 18446744073709551611.
ueqword CStringUtils::ToQWord(const std::string& valor)
{
    const char caracteresValidos[] = CARACTERES_DECIMAIS_LITERAL;
    return ToUnsigned(valor, 10, caracteresValidos, std::numeric_limits<ueqword>::max());
}

// wasm func 1523 (srcloc line 739). Observed executing (typed candidate numbers, RDV checks).
uedword CStringUtils::ToDWord(const std::string& valor)
{
    const char caracteresValidos[] = CARACTERES_DECIMAIS_LITERAL;
    return ToUnsigned(valor, 10, caracteresValidos, std::numeric_limits<uedword>::max());
}

// wasm func 3507 (srcloc line 744). strtoull with base 16 accepts an optional "0x"/"0X" prefix.
uedword CStringUtils::HexToInt(const std::string& valor)
{
    const char caracteresValidos[] = CARACTERES_HEXADECIMAIS_LITERAL;
    return ToUnsigned(valor, 16, caracteresValidos, std::numeric_limits<uedword>::max());
}

// srcloc line 754 - no out-of-line copy: inlined into comum::CCalculaCV's MAC routine (wasm func 3700),
// which reads the 8-byte SipHash tag back as a big-endian number.
ueqword CStringUtils::HexToQWord(const std::string& valor)
{
    const char caracteresValidos[] = CARACTERES_HEXADECIMAIS_LITERAL;
    return ToUnsigned(valor, 16, caracteresValidos, std::numeric_limits<ueqword>::max());
}

// wasm func 3506 (srcloc line 808). Observed executing.
// Users: the BU QR-code signer (hex of the last SHA-512 -> 64 bytes to sign, func 5604),
// ecourna::app::dados::CSerialMidia, mock data (func 9963), func 13441.
// An odd trailing nibble is silently dropped (the loop stops at size-1).
std::vector<uebyte> CStringUtils::HexStringToBytes(const std::string& valor)
{
    std::string texto = valor;
    Trim(texto);
    if (texto.empty())
        return {};
    if (!std::all_of(texto.begin(), texto.end(), [](char c) { return std::isxdigit(c) != 0; }))   // musl isxdigit = func 2965
        throw CUtilError(EUtilError::HexadecimalInvalido, "valor invalido [" + texto + "]");      // quotes the TRIMMED text

    std::vector<uebyte> bytes;
    bytes.reserve(texto.size() / 2);
    for (int i = 0; i < static_cast<int>(texto.size()) - 1; i += 2) {
        const int alto = ValorHexa(texto[i]);
        const int baixo = ValorHexa(texto[i + 1]);
        if (alto >= 0 && baixo >= 0)            // always true after the all_of check (dead test)
            bytes.push_back(static_cast<uebyte>((alto << 4) + baixo));
    }
    return bytes;
}

// ------------------------------------------------------------------------------------------------------
// Text transforms
// ------------------------------------------------------------------------------------------------------

// wasm func 1879 - name inferred. Observed executing.
// Copy + in-place transform: an instance of the merge-similar body func 3942 (copy, then invoke the
// transform through table slot 6425 = func 5158). Callers: CSqlResultSet (column names),
// comum::(anonymous)::FormataUF (file names use the lower-case UF), funcs 3753, 5726, api 5728.
std::string CStringUtils::ToLower(const std::string& texto)
{
    std::string copia = texto;
    ToLower(copia);   // func 5158: Latin-1 aware - the 19 upper-case accented letters of the static
                      // set @1911836 (Ç Á É Í Ó Ú À È Ì Ò Ù Â Ê Î Ô Û Ã Õ Ñ, built on first use) get |= 0x20,
                      // other letters go through isalpha/tolower.
    return copia;
}

// wasm func 9406 - name inferred. Observed executing.
// In-place trim of every byte <= ' ' (spaces AND control characters) at both ends. Used by all numeric
// conversions and, through the copying variant Trim(const std::string&) (func 1374 -> slot 6416), by comum.
void CStringUtils::Trim(std::string& texto)
{
    if (texto.empty())
        return;
    const auto naoBranco = [](unsigned char c) { return c > ' '; };
    const auto fim = std::find_if(texto.rbegin(), texto.rend(), naoBranco).base();
    texto.erase(fim, texto.end());
    const auto inicio = std::find_if(texto.begin(), texto.end(), naoBranco);
    texto.erase(texto.begin(), inicio);
}

// Members owned by other units (listed for completeness):
//   Trim(const std::string&) func 1374, ToUpper(const std::string&) func 5156, ToUpper(std::string&) func 3509,
//   ToLower(std::string&) func 5158, the lazy initialiser of its accent set func 5157, Replace func 9398.

} // namespace ecourna::api::util
