// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original (path inferred): uenux2/src/app/comum/dados/md/cregracpf.cpp
// Class comum::md::CRegraCPF (declared in iregraidentidade.h of this reconstruction; the original
// probably had cregracpf.h). CPF = Cadastro de Pessoas Físicas, 11 digits, two mod-11 check digits.
#include "iregraidentidade.h"

#include <array>

namespace comum::md {

// wasm func 11269 (vtable slot 3)
std::string CRegraCPF::Formata(const std::string& identidade) const
{
    return api::CStringUtils::PadLeft(identidade, '0', 11);   // api_f753
}

// wasm func 11270 (vtable slot 4)
// Standard CPF check-digit algorithm. The caller (IRegraIdentidade::EhValida) already guaranteed
// that the string is non-empty and digits-only, and stripped its leading zeros.
bool CRegraCPF::Verifica(const std::string& identidade) const
{
    if (identidade.size() > 11)
        return false;

    const std::string cpf = api::CStringUtils::PadLeft(identidade, '0', 11);
    std::array<std::uint8_t, 11> d{};
    for (std::size_t i = 0; i < cpf.size(); ++i)
        d[i] = static_cast<std::uint8_t>(cpf[i] - '0');

    std::uint16_t soma = 0;
    for (int i = 0; i < 9; ++i) soma += d[i] * (10 - i);           // weights 10..2
    std::uint8_t resto = soma % 11;
    const std::uint8_t dv1 = resto >= 2 ? 11 - resto : 0;
    if (d[9] != dv1)
        return false;

    soma = 0;
    for (int i = 0; i < 9; ++i) soma += d[i] * (11 - i);           // weights 11..3
    soma += dv1 * 2;
    resto = soma % 11;
    const std::uint8_t dv2 = resto >= 2 ? 11 - resto : 0;
    return d[10] == dv2;
    // NOTE: repeated-digit CPFs (000.000.000-00, 111.111.111-11, ...) pass this check.
}

}  // namespace comum::md
