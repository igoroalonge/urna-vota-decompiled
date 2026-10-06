// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.h
// md side of ModuloComplementosMunicipios::ComplementoMunicipio.
#pragma once

#include <cstdint>
#include <optional>

#include "../chorarioverao.h"

namespace comum::md {

class CComplementoMunicipio {
public:
    const CHorarioVerao& GetHorarioVerao() const;    // func 2795 (:37)
    void ValidaCriacao() const;                      // func 5647 (:47, :52)

private:
    std::uint32_t m_codigoMunicipio;                 // +0 (1..99999)
    std::int16_t m_fuso;                             // +4 minutes (-720..720)
    bool m_desligaColetaBiometria;                   // +6 ?
    std::optional<CHorarioVerao> m_horarioVerao;     // +8 (engaged +28)
};

}  // namespace comum::md
