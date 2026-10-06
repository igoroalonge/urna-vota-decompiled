// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/estadoaplicacao/clocalidadeeleitoral.h
#pragma once

#include <cstdint>

namespace comum::md::estadoaplicacao {

class CLocalidadeEleitoral {
public:
    CLocalidadeEleitoral(std::uint32_t municipio, std::uint16_t zona, std::uint16_t secao);   // func 5631
    void ValidaCriacao() const;                                                              // inlined
private:
    std::uint32_t m_municipio;   // +0
    std::uint16_t m_zona;        // +4
    std::uint16_t m_secao;       // +6
};

}  // namespace comum::md::estadoaplicacao
