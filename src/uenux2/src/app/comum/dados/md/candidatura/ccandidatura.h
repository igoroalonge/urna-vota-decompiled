// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/candidatura/ccandidatura.h
#pragma once

#include <cstdint>
#include <vector>

#include "cdadoscandidato.h"   // comum::md::CDadosCandidato (52 bytes)

namespace comum::md {

using TCargoID = std::uint8_t;
using TPartidoID = std::uint16_t;
using TCandidatoID = std::uint32_t;
using uebyte = std::uint8_t;

class CCandidatura {
public:
    CCandidatura(const TCargoID cargo, const TPartidoID partido, const TCandidatoID numero,
                 const CDadosCandidato& titular, const std::vector<CDadosCandidato>& suplentes);   // func 5660
    const CDadosCandidato& GetSuplente(uebyte ordem) const;                                         // func 1389

private:
    TCargoID m_cargo;                          // +0
    TPartidoID m_partido;                      // +2
    TCandidatoID m_numero;                     // +4
    CDadosCandidato m_titular;                 // +8
    std::vector<CDadosCandidato> m_suplentes;  // +60 (api_f2870 = vector range-copy)
};

}  // namespace comum::md
