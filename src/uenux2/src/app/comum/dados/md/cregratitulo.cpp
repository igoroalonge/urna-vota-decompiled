// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original (path inferred): uenux2/src/app/comum/dados/md/cregratitulo.cpp
// Class comum::md::CRegraTitulo: título de eleitor = 8-digit sequence + 2-digit UF code (01..28,
// 28 = ZZ/exterior) + 2 mod-11 check digits. Observed executing during the recorded votes.
#include "iregraidentidade.h"

#include <array>

namespace comum::md {

namespace {
// Remainder -> check digit. Remainders 0 and 1 give 0, except for UF codes 01 (SP) and 02 (MG)
// where remainder 0 gives 1 (the wasm computes "resto ^ 1" for those states).
std::uint8_t DigitoVerificador(std::uint8_t resto, bool spOuMg)
{
    if (resto < 2)
        return spOuMg ? (resto ^ 1) : 0;
    return 11 - resto;
}
}  // namespace

// wasm func 11271 (vtable slot 3)
std::string CRegraTitulo::Formata(const std::string& identidade) const
{
    return api::CStringUtils::PadLeft(identidade, '0', 12);   // api_f753
}

// wasm func 11272 (vtable slot 4)
bool CRegraTitulo::Verifica(const std::string& identidade) const
{
    if (identidade.size() > 12)
        return false;

    const std::string titulo = api::CStringUtils::PadLeft(identidade, '0', 12);
    std::array<std::uint8_t, 12> d{};
    for (std::size_t i = 0; i < titulo.size(); ++i)
        d[i] = static_cast<std::uint8_t>(titulo[i] - '0');

    const int uf = d[8] * 10 + d[9];
    if (uf < 1 || uf > 28)                        // compiled as (uf - 29) <u -28
        return false;
    const bool spOuMg = uf < 3;

    // weights 9..2 (the TSE formula with weights 2..9 is the same modulo 11 - see u05 doc)
    std::uint16_t soma = 0;
    for (int i = 0; i < 8; ++i) soma += d[i] * (9 - i);
    if (d[10] != DigitoVerificador(soma % 11, spOuMg))
        return false;

    const std::uint16_t soma2 = d[8] * 4 + d[9] * 3 + d[10] * 2;
    return d[11] == DigitoVerificador(soma2 % 11, spOuMg);
}

}  // namespace comum::md
