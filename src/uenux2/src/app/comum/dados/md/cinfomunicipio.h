// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/cinfomunicipio.h
#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "chorarioverao.h"   // ? comum::md::CHorarioVerao {inicio, fim, diferenca}

namespace comum::md {

class CInfoMunicipio {
public:
    const CHorarioVerao& GetHorarioVerao() const;   // func 3725
    void ValidaCriacao() const;                     // func 5675

private:
    std::uint32_t m_codigo;                         // +0
    std::string m_nome;                             // +4
    std::int16_t m_fuso;                            // +16 minutes
    std::optional<CHorarioVerao> m_horarioVerao;    // +20 (engaged +40)
};

}  // namespace comum::md
