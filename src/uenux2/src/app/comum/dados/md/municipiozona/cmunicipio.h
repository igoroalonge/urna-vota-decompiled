// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/municipiozona/cmunicipio.h
#pragma once

#include <cstdint>
#include <string>

namespace comum::md {

using TMunicipioID = std::uint32_t;

class CMunicipio {
public:
    CMunicipio(TMunicipioID codigo, const std::string& nome, bool comBiometria);   // func 3712 (:27)
    TMunicipioID GetCodigo() const { return m_codigo; }

private:
    TMunicipioID m_codigo;   // +0
    std::string m_nome;      // +4 (trimmed)
    bool m_comBiometria;     // +16 (ASN.1 Municipio.comBiometria [2] OPTIONAL, default false)
};

}  // namespace comum::md
