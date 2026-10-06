// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeral.h
// Only RecuperarCertificado belongs to unit u05; the rest of CEstadoGeral (eg.bin state) is elsewhere.
#pragma once

#include <cstdint>
#include <vector>

namespace comum::md::estadoaplicacao {

using uebyte = std::uint8_t;

class CEstadoGeral {
public:
    static std::vector<uebyte> RecuperarCertificado();   // func 5635
};

}  // namespace comum::md::estadoaplicacao
