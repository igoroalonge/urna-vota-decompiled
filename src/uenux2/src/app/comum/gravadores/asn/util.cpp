// uenux2/src/app/comum/gravadores/asn/util.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// Serial number of a flash card ("serial da MV/MC"): 8 hex characters in the C++ model, 4 raw bytes
// (OCTET STRING (SIZE(4))) in the ASN.1 entities (Urna.numeroSerieFV, Carga.numeroSerieFC, ...).
// Both functions ran during the recorded votes: DesconverteSerialFlash when the state files are LOADED
// (caller 11400, CConversorDadoCorrespondencia::DoDesconverte), ConverteSerialFlash when they are SAVED
// (caller 11399, CConversorDadoCorrespondencia::DoConverte).
#include <cctype>
#include <format>
#include <string>
#include <vector>

#include "comum/gravadores/iresultado.h"          // CUeComumGravadoresError
#include "ecourna/api/util/cstringutils.h"

namespace comum::asn {

namespace {
bool EhHexa(char c) { return std::string_view("0123456789ABCDEFabcdef").find(c) != std::string_view::npos; }  // memchr @169476
}

// wasm func 3605 (srcloc util.cpp:26, :31)
std::vector<char> ConverteSerialFlash(const std::string& serial)
{
    if (serial.size() != 8)
        throw CUeComumGravadoresError(8631, "Serial inválido [" + serial + "]");            // :26
    for (int i = 0; i < 8; ++i)                                                            // unrolled 8 x memchr
        if (!EhHexa(serial[i]))
            throw CUeComumGravadoresError(8632, "Serial inválido [" + serial + "]");        // :31

    std::vector<char> bytes;
    for (std::size_t i = 0; i < 4; ++i) {
        const char par[3] = {serial[2 * i], serial[2 * i + 1], '\0'};
        bytes.push_back(static_cast<char>(ecourna::api::util::CStringUtils::HexToInt(std::string(par))));
    }
    return bytes;
}

// wasm func 3604 (srcloc util.cpp:54)
std::string DesconverteSerialFlash(const std::vector<char>& bytes)
{
    std::string serial;
    // NOTE (order as compiled): an empty vector throws immediately; otherwise every byte is formatted first
    // and the size is checked afterwards, so the message shows the hex text built so far, not the input.
    if (!bytes.empty()) {
        for (std::size_t i = 0; i < bytes.size(); ++i)
            serial += std::format("{:02X}", static_cast<unsigned char>(bytes[i]));
        if (bytes.size() == 4)
            return serial;
    }
    throw CUeComumGravadoresError(8633, "Serial inválido [" + serial + "]");                // :54
}

}  // namespace comum::asn
