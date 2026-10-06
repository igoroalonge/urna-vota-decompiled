// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/chorarioveraomunicipio.h
#pragma once

#include <cstdint>

#include "chorarioverao.h"

namespace comum::md {

class CHorarioVeraoMunicipio {
public:
    void ValidaCriacao() const;          // func 5678
private:
    std::uint32_t m_codigoMunicipio;     // +0
    CHorarioVerao m_horarioVerao;        // +4 ?
};

}  // namespace comum::md
