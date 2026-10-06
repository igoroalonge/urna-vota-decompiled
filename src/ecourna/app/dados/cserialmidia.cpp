// ecourna-lib/ecourna/app/dados/cserialmidia.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/cserialmidia.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// Encoding note: the error message is stored in ISO-8859-1 in the binary ("N\xFAmero de s\xE9rie ...",
// string @367508), like almost every literal of ecourna/app/dados (the source files are Latin-1).
// The one exception in this unit is cconversorturno.cpp, whose "Turno inválido." is UTF-8.
#include "ecourna/app/dados/cserialmidia.h"

#include <format>
#include <regex>

#include "ecourna/api/util/cstringutils.h"
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

namespace {

// wasm func 9258 (no srcloc; name inferred). std::regex(pattern) is func 2416 (flags 512 =
// ECMAScript in libc++ ABI v2); std::regex_match over the whole string is func 9517.
// The regex object is built on every call (not static).
bool SerialValido(const std::string& serial)
{
    return std::regex_match(serial, std::regex("([A-Fa-f0-9]{8})"));
}

} // namespace

// wasm func 9259 (srcloc line 37)
CSerialMidia::CSerialMidia(const std::string& serial)
{
    if (!SerialValido(serial)) {
        throw CDadosError(1951, std::format("Número de série '{}' inválido.", serial));   // line 37
    }
    m_serial = api::util::CStringUtils::HexStringToBytes(serial);                     // table slot 467
}

} // namespace ecourna::app::dados
